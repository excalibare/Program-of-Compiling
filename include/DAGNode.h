// DAGNode.h
#ifndef DAGNODE_H
#define DAGNODE_H

#include <string>
#include <vector>
#include <set>
#include <memory> // For std::shared_ptr
#include <algorithm> // For std::remove, std::find

// 前向声明 DAGNode 以在 DAGNodePtr 中使用
struct DAGNode;
// 为 DAGNode 的智能指针定义类型别名
using DAGNodePtr = std::shared_ptr<DAGNode>;

struct DAGNode {
    int id;                     // 节点唯一的ID
    std::string op_label;       // 节点代表的操作（如 "+", "*", "ident", "const", "UMINUS" 等）
    std::string value;          // 如果是叶子节点 (ident 或 const)，这里存储其名称或值
    std::vector<DAGNodePtr> children; // 指向子节点的指针列表 (对于运算节点)
    std::set<std::string> identifiers; // 存储所有当前值由该节点表示的变量名和临时变量名

    bool is_operator_node;   // 标记此节点是否为运算节点 (true) 或叶子节点 (false)

    // 用于常量折叠的结果
    bool is_constant_val;       // 标记此节点的值是否为一个编译时常量
    std::string constant_value; // 如果 is_constant_val 为 true，这里存储常量的值

    // 构造函数
    DAGNode(int id, const std::string& op_lbl, const std::string& val = "", bool is_op = false)
        : id(id), op_label(op_lbl), value(val), is_operator_node(is_op),
          is_constant_val(false), constant_value("") {}

    // 辅助函数：比较此运算节点是否与给定的操作和子节点相同（用于查找公共子表达式）
    bool equalsOp(const std::string& other_op_label, const std::vector<DAGNodePtr>& other_children) const {
        if (!is_operator_node || op_label != other_op_label || children.size() != other_children.size()) {
            return false;
        }
        // 比较子节点时，我们比较子节点的ID，因为它们应该是DAG中唯一的节点
        for (size_t i = 0; i < children.size(); ++i) {
            if (!children[i] || !other_children[i] || children[i]->id != other_children[i]->id) {
                return false;
            }
        }
        return true;
    }

    // 辅助函数：比较此叶子节点是否与给定的标签和值相同
    bool equalsLeaf(const std::string& other_label, const std::string& other_value) const {
        return !is_operator_node && op_label == other_label && value == other_value;
    }

    // 添加一个变量/临时变量名到此节点的标识符集合
    void addIdentifier(const std::string& id_name) {
        identifiers.insert(id_name);
    }

    //从此节点的标识符集合中移除一个变量/临时变量名
    void removeIdentifier(const std::string& id_name) {
        identifiers.erase(id_name);
    }
};

#endif // DAGNODE_H