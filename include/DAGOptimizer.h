// DAGOptimizer.h
#ifndef DAGOPTIMIZER_H
#define DAGOPTIMIZER_H

#include "Parser.h" // 假设 Parser.h 定义了 Parser::Quad
#include "DAGNode.h"  // 引入我们上面定义的 DAGNode 结构
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory> // For std::shared_ptr

// 使用 Parser::Quad 作为 Quadruple 的类型别名
// 如果您的 Quad 定义在别处或名称不同，请相应修改
using Quadruple = Parser::Quad;

class DAGOptimizer {
public:
    // 构造函数，接收原始的四元式序列
    DAGOptimizer(const std::vector<Quadruple>& input_quads);

    // 构建并优化DAG
    void buildAndOptimizeDAG();
    // 从优化后的DAG重构四元式序列
    void reconstructQuadsFromDAG();

    // 获取优化后的四元式序列
    const std::vector<Quadruple>& getOptimizedQuads() const;

    // (可选) 打印DAG结构以供调试
    void printDAG() const;

private:
    std::vector<Quadruple> original_quads_; // 存储输入的原始四元式
    std::vector<Quadruple> optimized_quads_; // 存储优化后重构的四元式

    std::vector<DAGNodePtr> dag_nodes_; // 存储DAG中所有唯一的节点
    // 核心数据结构：映射变量名（或临时变量名）到当前代表其值的DAG节点
    std::map<std::string, DAGNodePtr> active_node_for_var_;

    int next_node_id_; // 用于为新创建的DAG节点分配唯一ID
    int temp_var_counter_for_recon_; // 用于在重构四元式时生成新的临时变量名 (如 RT0, RT1)

    // --- 辅助函数 ---
    // 获取或创建叶子节点 (用于常量或标识符)
    DAGNodePtr getLeafNode(const std::string& value_or_identifier_name);
    // 获取或创建运算节点 (用于各种运算)
    // arg2_node 对于一元运算可以为 nullptr
    DAGNodePtr getOperatorNode(const std::string& op, DAGNodePtr arg1_node, DAGNodePtr arg2_node = nullptr);

    // 判断字符串是否为数字常量
    bool isNumericConstant(const std::string& s) const;
    // 计算常量表达式的值，如果操作数不是常量或无法计算，则返回空字符串
    // val2_str 对于一元运算可以为空
    std::string evaluateConstantExpression(const std::string& op, const std::string& val1_str, const std::string& val2_str = "");

    // --- 四元式重构相关的辅助函数 ---
    // 递归地从DAG节点生成四元式
    void generateQuadsRecursive(DAGNodePtr node,
                                std::map<int, std::string>& node_id_to_result_var,
                                std::set<int>& visited_nodes);
    // (已修正) 获取给定DAG节点计算结果所对应的变量名（通常来自 node_id_to_result_var 映射）
    std::string getResultVarForNode(DAGNodePtr node,
                                    std::map<int, std::string>& node_id_to_result_var);
    // 为重构阶段生成新的临时变量名 (RT0, RT1, ...)
    std::string newTempRecon();

    // 用于存储那些不直接参与表达式DAG构建的四元式（如控制流、直接IO）
    // 这些指令会在重构时被重新插入，其参数可能会被优化
    std::vector<Quadruple> control_flow_and_io_quads_;
};

#endif // DAGOPTIMIZER_H