// DAGOptimizer.cpp
#include "DAGOptimizer.h"
#include <iostream>     // 用于 printDAG 和调试输出
#include <stdexcept>    // 用于 std::stod, std::runtime_error (如果需要)
#include <sstream>      // 用于 std::ostringstream (数字转字符串)
#include <algorithm>    // 用于 std::all_of (在 isNumericConstant 中可以考虑)

// --- 构造函数 ---
DAGOptimizer::DAGOptimizer(const std::vector<Quadruple>& input_quads)
    : original_quads_(input_quads), next_node_id_(0), temp_var_counter_for_recon_(0) {
}

// --- 工具函数 ---
bool DAGOptimizer::isNumericConstant(const std::string& s) const {
    if (s.empty()) {
        return false;
    }
    char* end = nullptr;
    std::strtod(s.c_str(), &end); // 尝试转换为 double
    // 如果转换成功（end 指向了字符串末尾的 '\0'），并且字符串非空，则认为是数字
    return end != s.c_str() && *end == '\0';
}

std::string DAGOptimizer::evaluateConstantExpression(const std::string& op, const std::string& val1_str, const std::string& val2_str) {
    // 确保操作数是数字常量
    if (!isNumericConstant(val1_str) || (!val2_str.empty() && !isNumericConstant(val2_str))) {
        return ""; // 操作数不全是常量，无法折叠
    }

    double val1 = std::stod(val1_str);
    double val2 = val2_str.empty() ? 0.0 : std::stod(val2_str); // 一元运算时 val2 可视为0或不使用
    double result_val;

    if (op == "+") result_val = val1 + val2;
    else if (op == "-") result_val = val1 - val2; // 注意：这里处理的是二元减法
    else if (op == "*") result_val = val1 * val2;
    else if (op == "/") {
        if (val2 == 0.0) return ""; // 除零错误
        result_val = val1 / val2;
    } else if (op == "UMINUS") { // 特殊处理一元负号
        result_val = -val1;
    }
    // TODO: 可以添加更多关系运算符的常量折叠 (结果为 "0" 或 "1")
    // else if (op == "<") result_val = (val1 < val2);
    // ...
    else {
        return ""; // 未知或不可折叠的运算符
    }

    // 将结果转换为字符串，并尽量美化（例如，整数不带 .0）
    std::ostringstream oss;
    oss << result_val;
    std::string result_str = oss.str();
    // 尝试移除不必要的 ".0" 或 ".00"
    size_t decimal_pos = result_str.find('.');
    if (decimal_pos != std::string::npos) {
        bool all_zeros_after_decimal = true;
        for (size_t i = decimal_pos + 1; i < result_str.length(); ++i) {
            if (result_str[i] != '0') {
                all_zeros_after_decimal = false;
                break;
            }
        }
        if (all_zeros_after_decimal) {
            result_str = result_str.substr(0, decimal_pos);
            if (result_str.empty() && result_val == 0) result_str = "0"; // 处理 "0.0" -> "0"
        }
    }
    return result_str;
}

DAGNodePtr DAGOptimizer::getLeafNode(const std::string& value_or_identifier_name) {
    std::string label = isNumericConstant(value_or_identifier_name) ? "const" : "ident";
    // 查找是否已存在相同的叶子节点
    for (const auto& node : dag_nodes_) {
        if (node->equalsLeaf(label, value_or_identifier_name)) {
            return node; // 已存在，直接返回
        }
    }
    // 不存在，创建新的叶子节点
    DAGNodePtr new_node = std::make_shared<DAGNode>(next_node_id_++, label, value_or_identifier_name, false);
    if (label == "const") {
        new_node->is_constant_val = true;
        new_node->constant_value = value_or_identifier_name;
    }
    dag_nodes_.push_back(new_node);
    return new_node;
}

DAGNodePtr DAGOptimizer::getOperatorNode(const std::string& op, DAGNodePtr arg1_node, DAGNodePtr arg2_node) {
    if (!arg1_node) { // 至少需要一个操作数（对于一元或二元运算）
        // std::cerr << "Error: getOperatorNode called with null arg1_node for op: " << op << std::endl;
        // 返回一个表示错误的特殊节点或抛出异常，这里简单返回nullptr示意
        return nullptr;
    }

    std::vector<DAGNodePtr> children;
    children.push_back(arg1_node);
    if (arg2_node) { // arg2_node 存在，则为二元运算
        children.push_back(arg2_node);
    }

    // 1. 尝试常量折叠
    std::string folded_const_val;
    if (op != ":=") { // 赋值运算符本身不参与数值上的折叠
        if (children.size() == 1 && arg1_node->is_constant_val) { // 一元运算
            folded_const_val = evaluateConstantExpression(op, arg1_node->constant_value);
        } else if (children.size() == 2 && arg1_node->is_constant_val && arg2_node && arg2_node->is_constant_val) { // 二元运算
            folded_const_val = evaluateConstantExpression(op, arg1_node->constant_value, arg2_node->constant_value);
        }
    }

    if (!folded_const_val.empty()) { // 常量折叠成功
        return getLeafNode(folded_const_val); // 返回代表结果常量的叶子节点
    }

    // 2. 查找公共子表达式
    for (const auto& node : dag_nodes_) {
        if (node->equalsOp(op, children)) {
            return node; // 找到相同的运算节点，直接返回
        }
    }

    // 3. 创建新的运算节点
    DAGNodePtr new_node = std::make_shared<DAGNode>(next_node_id_++, op, "", true);
    new_node->children = children;
    dag_nodes_.push_back(new_node);
    return new_node;
}

// --- DAG 构建与优化 ---
void DAGOptimizer::buildAndOptimizeDAG() {
    dag_nodes_.clear();
    active_node_for_var_.clear();
    control_flow_and_io_quads_.clear();
    next_node_id_ = 0;

    for (const auto& quad : original_quads_) {
        DAGNodePtr node_arg1 = nullptr;
        DAGNodePtr node_arg2 = nullptr;
        DAGNodePtr result_node = nullptr;
        std::string current_op = quad.op;

        // --- 首先处理特殊的四元式 (IO, 控制流) ---
        if (current_op == "READ") {
            if (active_node_for_var_.count(quad.result)) {
                active_node_for_var_[quad.result]->removeIdentifier(quad.result);
            }
            result_node = getLeafNode(quad.result);
            if(result_node) { // 确保 getLeafNode 成功
                result_node->addIdentifier(quad.result);
                result_node->is_constant_val = false;
                active_node_for_var_[quad.result] = result_node;
            }
            control_flow_and_io_quads_.push_back(quad);
            continue;
        } else if (current_op == "WRITE") {
            Quadruple quad_to_store = quad;
            if (!quad.arg1.empty()) {
                DAGNodePtr value_node_for_write = nullptr;
                if (active_node_for_var_.count(quad.arg1)) {
                    value_node_for_write = active_node_for_var_[quad.arg1];
                } else if (isNumericConstant(quad.arg1)) {
                    value_node_for_write = getLeafNode(quad.arg1);
                }

                if (value_node_for_write && value_node_for_write->is_constant_val) {
                    quad_to_store.arg1 = value_node_for_write->constant_value;
                }
            }
            control_flow_and_io_quads_.push_back(quad_to_store);
            continue;
        } else if (current_op == "JMP" || current_op == "JPC" /* || 其他控制流指令 */) {
            // TODO: JPC 的条件参数 quad.arg1 也可能从 active_node_for_var_ 获取其对应的常量值（如果适用）
            Quadruple quad_to_store_jmp = quad;
            if (current_op == "JPC" && !quad.arg1.empty()) {
                 DAGNodePtr cond_node = nullptr;
                 if (active_node_for_var_.count(quad.arg1)) {
                    cond_node = active_node_for_var_[quad.arg1];
                 } else if (isNumericConstant(quad.arg1)) {
                    cond_node = getLeafNode(quad.arg1);
                 }
                 if (cond_node && cond_node->is_constant_val) {
                    quad_to_store_jmp.arg1 = cond_node->constant_value;
                 }
            }
            control_flow_and_io_quads_.push_back(quad_to_store_jmp);
            continue;
        }

        // --- 处理参与DAG构建的四元式 (算术, 赋值, ODD 等) ---
        // 获取第一个操作数节点
        if (!quad.arg1.empty()) {
            if (active_node_for_var_.count(quad.arg1)) {
                node_arg1 = active_node_for_var_[quad.arg1];
            } else {
                node_arg1 = getLeafNode(quad.arg1);
                if (node_arg1 && node_arg1->op_label == "ident") { // 如果是新标识符，将其加入active_node_for_var_
                    active_node_for_var_[quad.arg1] = node_arg1;
                    node_arg1->addIdentifier(quad.arg1);
                }
            }
        }

        // 特殊处理一元负号： (-, 0, A, Res) -> (UMINUS, A, -, Res)
        if (current_op == "-" && quad.arg1 == "0" && !quad.arg2.empty()) {
            current_op = "UMINUS"; // 内部表示为 UMINUS
            if (active_node_for_var_.count(quad.arg2)) {
                node_arg1 = active_node_for_var_[quad.arg2]; // 实际操作数是 quad.arg2
            } else {
                node_arg1 = getLeafNode(quad.arg2);
                if (node_arg1 && node_arg1->op_label == "ident") {
                    active_node_for_var_[quad.arg2] = node_arg1;
                    node_arg1->addIdentifier(quad.arg2);
                }
            }
            node_arg2 = nullptr; // UMINUS 是一元运算
        } else if (!quad.arg2.empty()) { // 获取第二个操作数节点 (如果存在且不是上述特殊一元负号)
            if (active_node_for_var_.count(quad.arg2)) {
                node_arg2 = active_node_for_var_[quad.arg2];
            } else {
                node_arg2 = getLeafNode(quad.arg2);
                if (node_arg2 && node_arg2->op_label == "ident") {
                    active_node_for_var_[quad.arg2] = node_arg2;
                    node_arg2->addIdentifier(quad.arg2);
                }
            }
        }

        // 根据操作符获取或创建结果节点
        if (current_op == ":=") { // 赋值语句 (Result := Arg1)
            if(node_arg1) result_node = node_arg1; // 结果变量指向源操作数所在的节点
            else { /*std::cerr << "Warning: Assignment source '" << quad.arg1 << "' has no node." << std::endl;*/ }
        } else if (current_op == "ODD" || current_op == "UMINUS") { // 其他一元运算
            result_node = getOperatorNode(current_op, node_arg1);
        } else { // 二元运算 (+, *, <, == etc.)
            result_node = getOperatorNode(current_op, node_arg1, node_arg2);
        }

        // 更新结果变量到节点的映射
        if (!quad.result.empty() && result_node) {
            // 如果结果变量之前已映射到其他节点，移除旧的关联
            if (active_node_for_var_.count(quad.result)) {
                active_node_for_var_[quad.result]->removeIdentifier(quad.result);
            }
            result_node->addIdentifier(quad.result); // 将此结果变量与新计算得到的节点关联
            active_node_for_var_[quad.result] = result_node; // 更新 active_node_for_var_
        } else if (!quad.result.empty() && !result_node) {
            // std::cerr << "Warning: Quad result '" << quad.result << "' for op '" << current_op << "' has no result_node." << std::endl;
        }
    }
}

// --- 四元式重构 ---
std::string DAGOptimizer::newTempRecon() {
    return "RT" + std::to_string(temp_var_counter_for_recon_++);
}

// (已修正)
std::string DAGOptimizer::getResultVarForNode(DAGNodePtr node, std::map<int, std::string>& node_id_to_result_var) {
    if (!node) {
        // std::cerr << "Error: getResultVarForNode called with null node." << std::endl;
        return "ERROR_NULL_NODE_IN_GETVAR";
    }
    if (node_id_to_result_var.count(node->id)) {
        return node_id_to_result_var.at(node->id);
    }
    // 如果是叶子节点，并且还没有在 map 中（例如直接作为操作数使用），则返回其自身的值/名称
    if (!node->is_operator_node) {
        return node->value;
    }
    // std::cerr << "Error: Result variable name not found in map for node ID: " << node->id << " Op: " << node->op_label << std::endl;
    return "UNMAPPED_NODE_" + std::to_string(node->id); // 应该在 generateQuadsRecursive 中被赋值
}

void DAGOptimizer::generateQuadsRecursive(DAGNodePtr node,
                                          std::map<int, std::string>& node_id_to_result_var,
                                          std::set<int>& visited_nodes) {
    if (!node || visited_nodes.count(node->id)) {
        return; // 节点为空或已访问
    }
    visited_nodes.insert(node->id);

    // 如果是叶子节点 (常量或标识符)，将其值/名称存入映射，不生成计算它的四元式
    if (!node->is_operator_node) {
        node_id_to_result_var[node->id] = node->value;
        return;
    }

    // 递归处理子节点
    for (const auto& child : node->children) {
        generateQuadsRecursive(child, node_id_to_result_var, visited_nodes);
    }

    // 子节点处理完毕，获取子节点结果的变量名
    std::string arg1_name, arg2_name;
    if (!node->children.empty() && node->children[0]) {
        arg1_name = getResultVarForNode(node->children[0], node_id_to_result_var);
    } else if (!node->children.empty() && !node->children[0]) {
        // std::cerr << "Error: Node " << node->id << " child 0 is null." << std::endl;
        arg1_name = "ERROR_NULL_CHILD1";
    }


    if (node->children.size() > 1 && node->children[1]) {
        arg2_name = getResultVarForNode(node->children[1], node_id_to_result_var);
    } else if (node->children.size() > 1 && !node->children[1]) {
        // std::cerr << "Error: Node " << node->id << " child 1 is null." << std::endl;
        arg2_name = "ERROR_NULL_CHILD2";
    }


    // 决定此节点计算结果存储的变量名
    // 优先使用节点关联的非临时用户变量名
    std::string result_name_for_this_node;
    bool found_user_var = false;
    for (const std::string& id : node->identifiers) {
        if (!id.empty() && id.rfind("T", 0) != 0 && id.rfind("RT", 0) != 0) { // 不是 T 或 RT 开头的
            result_name_for_this_node = id;
            found_user_var = true;
            break;
        }
    }
    if (!found_user_var) { // 如果没有关联的用户变量，则生成新的临时变量
        result_name_for_this_node = newTempRecon();
    }
    node_id_to_result_var[node->id] = result_name_for_this_node;

    // 生成此节点的四元式
    // 注意：赋值操作 (:=) 在DAG中通常表现为节点标识符的传递，不直接生成为运算四元式，
    // 最终的赋值由 reconstructQuadsFromDAG 主逻辑处理。
    // 这里主要生成计算型四元式。
    if (node->op_label != ":=") { // 赋值不在此生成
        if (node->op_label == "UMINUS") { // 特殊处理一元负号，符合Parser输出格式
            optimized_quads_.push_back({"-", "0", arg1_name, result_name_for_this_node});
        } else if (node->op_label == "ODD" ) { // ODD A, , Res
             optimized_quads_.push_back({node->op_label, arg1_name, "", result_name_for_this_node});
        }
        else { // 其他二元或可能的一元（如果arg2_name为空）
            optimized_quads_.push_back({node->op_label, arg1_name, arg2_name, result_name_for_this_node});
        }
    }
}


void DAGOptimizer::reconstructQuadsFromDAG() {
    optimized_quads_.clear();
    temp_var_counter_for_recon_ = 0;
    std::map<int, std::string> node_id_to_result_var; // 节点ID -> 其计算结果存储的变量名
    std::set<int> visited_nodes_for_gen;              // 已为其生成计算四元式的节点

    // 确定哪些节点的计算结果是最终需要的
    // 1. 被 WRITE 指令使用的变量对应的节点
    // 2. 在原始四元式序列中，赋值给用户变量 (非 Tn) 的最终节点
    // 3. JPC 等控制流指令的条件参数对应的节点
    std::set<DAGNodePtr> root_nodes_to_generate_for;

    for (const auto& q_io_ctrl : control_flow_and_io_quads_) {
        if ((q_io_ctrl.op == "WRITE" || q_io_ctrl.op == "JPC") && !q_io_ctrl.arg1.empty()) {
            if (active_node_for_var_.count(q_io_ctrl.arg1)) { // 如果是变量
                root_nodes_to_generate_for.insert(active_node_for_var_[q_io_ctrl.arg1]);
            } else if (isNumericConstant(q_io_ctrl.arg1)) { // 如果是常量，也需要一个节点表示它
                 root_nodes_to_generate_for.insert(getLeafNode(q_io_ctrl.arg1));
            }
        }
        // 如果 JPC 的 quad.arg1 是一个表达式的结果 Tn, 那么 Tn 应该在 active_node_for_var_ 中
    }

    // 遍历 active_node_for_var_ 中所有最终由用户变量持有的值
    // 这些是基本块的 "live-out" 变量（或我们假设它们是）
    for (const auto& pair_var_node : active_node_for_var_) {
        const std::string& var_name = pair_var_node.first;
        // 通常我们关心非临时变量的最终值
        if (!var_name.empty() && var_name.rfind("T", 0) != 0 && var_name.rfind("RT", 0) != 0) {
            root_nodes_to_generate_for.insert(pair_var_node.second);
        }
    }

    // 为所有需要计算的根节点生成计算它们的四元式
    for (DAGNodePtr root_node : root_nodes_to_generate_for) {
        if(root_node) generateQuadsRecursive(root_node, node_id_to_result_var, visited_nodes_for_gen);
    }

    // 处理最终的赋值：确保所有在 active_node_for_var_ 中的用户变量都被正确赋值
    // (如果它们的计算结果在一个临时变量 RTn 中)
    for (const auto& pair_var_node : active_node_for_var_) {
        const std::string& var_name = pair_var_node.first;
        DAGNodePtr node = pair_var_node.second;

        if (!node || (var_name.rfind("T", 0) == 0 && var_name.rfind("RT", 0) != 0)) {
            // 忽略原始临时变量 T 开头的（除非它们是 RTn），因为它们通常是中间结果
            // 但如果一个用户变量最终的值由一个T变量持有，这个逻辑需要调整
            // 这里主要目标是 UserVar := RTn 或 UserVar := Constant
            continue;
        }

        // 获取该节点计算结果的名称（可能是RTn, 原始变量, 或常量值）
        std::string computed_value_name = getResultVarForNode(node, node_id_to_result_var);

        // 如果变量名与计算值的名称不同，并且该变量不是计算值本身（例如 X 不是 RTn）
        // 并且 X 不是一个直接的常量值 (X := 5 应该由 generateQuadsRecursive 生成 5->X 的映射)
        // 且 X 不是 RT 开头的（RTn := ... 已经生成）
        bool needs_final_assignment = true;
        if (var_name == computed_value_name) {
            needs_final_assignment = false;
        }
        // 如果 var_name 是一个用户变量，而 computed_value_name 是一个代表该节点值的 RTn 或常量，
        // 并且还没有直接计算到 var_name 的四元式，则添加 X := RTn 或 X := const
        if (node->is_operator_node) { // 如果 node 是运算节点，它的结果可能在 RTn 或直接在 var_name
             // generateQuadsRecursive 会尝试直接使用 var_name 作为结果
             // 如果它用了 RTn， 那么 computed_value_name 会是 RTn
        } else { // 叶子节点 (常量或 ident)
            // 如果 var_name := constant, 且 computed_value_name 是该常量，则不需要额外赋值
            // 如果 var_name := other_ident, 且 computed_value_name 是 other_ident, 也不需要
            // 这种情况通常是 var_name 指向了代表常量的叶节点，或另一个ident的叶节点
        }


        // 检查是否已经存在一个将 computed_value_name 赋给 var_name 的四元式
        // 或者一个直接计算结果到 var_name 的四元式
        bool already_assigned_or_computed = false;
        for(const auto& q : optimized_quads_){
            if(q.result == var_name){
                if(q.op == ":=" && q.arg1 == computed_value_name) {already_assigned_or_computed = true; break;}
                if(q.op == node->op_label && node_id_to_result_var.count(node->id) && node_id_to_result_var[node->id] == var_name) {
                    // 如果这个节点的操作直接把结果存到了var_name，也算
                    already_assigned_or_computed = true; break;
                }
            }
        }
        if (var_name == computed_value_name) already_assigned_or_computed = true;


        if (needs_final_assignment && !already_assigned_or_computed &&
            !(var_name.rfind("RT",0) == 0) && // 不要生成 RTn := ... 的赋值
            computed_value_name != "UNMAPPED_NODE_" + std::to_string(node->id) && // 确保值有效
            computed_value_name != "ERROR_NULL_NODE_IN_GETVAR" ) {
            optimized_quads_.push_back({":=", computed_value_name, "", var_name});
        }
    }


    // 最后，按原序（或逻辑顺序）添加之前分离出来的控制流和IO四元式
    // 注意：它们的参数可能已经在 control_flow_and_io_quads_ 中被优化了
    for (const auto& q_io_ctrl : control_flow_and_io_quads_) {
        optimized_quads_.push_back(q_io_ctrl);
    }
}


// --- 获取结果 ---
const std::vector<Quadruple>& DAGOptimizer::getOptimizedQuads() const {
    return optimized_quads_;
}

// --- 调试用：打印DAG结构 ---
void DAGOptimizer::printDAG() const {
    std::cerr << "\n--- DAG Nodes List (" << dag_nodes_.size() << " nodes) ---" << std::endl;
    for (const auto& node : dag_nodes_) {
        if (!node) continue;
        std::cerr << "Node ID: " << node->id << "\t Op: '" << node->op_label << "'";
        if (!node->value.empty()) {
            std::cerr << "', Value: '" << node->value << "'";
        }
        if (node->is_constant_val) {
            std::cerr << " (Const: " << node->constant_value << ")";
        }
        std::cerr << "\t Identifiers: { ";
        for (const auto& id : node->identifiers) {
            std::cerr << id << " ";
        }
        std::cerr << "}";
        if (!node->children.empty()) {
            std::cerr << "\t Children IDs: [ ";
            for (const auto& child : node->children) {
                if (child) std::cerr << child->id << " ";
                else std::cerr << "null ";
            }
            std::cerr << "]";
        }
        std::cerr << std::endl;
    }

    std::cerr << "\n--- Active Variable Mappings (Var -> Node ID) ---" << std::endl;
    for (const auto& pair : active_node_for_var_) {
        if (pair.second) {
            std::cerr << "'" << pair.first << "' -> Node " << pair.second->id
                      << " (Op: " << pair.second->op_label;
            if(!pair.second->value.empty()) std::cerr << ", Val: " << pair.second->value;
            if(pair.second->is_constant_val) std::cerr << ", ConstVal: " << pair.second->constant_value;
            std::cerr << ")" << std::endl;
        } else {
            std::cerr << "'" << pair.first << "' -> null node" << std::endl;
        }
    }
    std::cerr << "--- End of DAG Print ---" << std::endl;
}