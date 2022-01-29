#ifndef NODE_HPP
#define NODE_HPP
#include "type.hpp"
class Node{
    public:
    int need_node_;
    static constexpr const char * NodeType_ = "node"; 
    std::string node_name_;
    std::string addr_name_;
    std::string index_name_;
    Type type_;
    void inc_need_node() {
        need_node_++;
    }
    void dec_need_node() {
        need_node_--;
    }
    int get_need_node() const{
        return need_node_;
    }
    Node():need_node_(0){
    }
    Node(const Type & type,const std::string & node_name ):
    need_node_(0),node_name_(node_name),type_(type)
    {
    }
    Type get_type()const {
        return type_;
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }
    virtual ~Node() = default;
};
class GatherNode : public Node{

    static constexpr const char * NodeType_ = "gather"; 
    public:
        Node * addr_ptr_;
    Node * index_ptr_;
    GatherNode(Node * addr_ptr, Node * index_ptr):addr_ptr_(addr_ptr), index_ptr_(index_ptr)
    {
        addr_ptr_->inc_need_node();
        index_ptr_->inc_need_node();
        type_ = *(addr_ptr->get_type().get_pointer2type());
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }

};
class ScatterNode :public Node{

    static constexpr const char * NodeType_ = "scatter"; 
    public:
    Node * addr_ptr_;
    Node * index_ptr_;
    Node * data_ptr_;
    ScatterNode(Node * addr_ptr, Node * index_ptr, Node * data_ptr):addr_ptr_(addr_ptr), index_ptr_(index_ptr),data_ptr_(data_ptr)
    {
        addr_ptr_->inc_need_node();
        index_ptr_->inc_need_node();

        data_ptr_->inc_need_node();
        type_ = *(addr_ptr->get_type().get_pointer2type());
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }

};
class VarNode :public Node{

    static constexpr const char * NodeType_ = "var"; 
    public:
    VarNode(const Type& type,const std::string &node_name) : Node( type,node_name ) {
    
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }

};
class LoadNode:public Node{

    static constexpr const char * NodeType_ = "load"; 
    public:
    Node * addr_ptr_;

    LoadNode(Node * addr_ptr):addr_ptr_(addr_ptr)
    {
        Type * pointer_type = addr_ptr->get_type().get_pointer2type();
        addr_ptr_->inc_need_node();


        if(pointer_type == nullptr) {
            LOG(FATAL) << "Wrong";
        } else 
            type_ = *(pointer_type);
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }

};
class BinaryNode:public  Node{

    static constexpr const char * NodeType_ = "binary"; 
    public:
    Node * left_node_;
    Node * right_node_;

    BinaryNode(Node * left_node,  Node * right_node):left_node_(left_node),right_node_(right_node)
    {

        left_node_->inc_need_node();
        right_node_->inc_need_node();
        type_ = left_node_->get_type();
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }

};
#define BINARY_DEFINE(CLASS_NAME) \
    class CLASS_NAME##Node :public BinaryNode { \
        static constexpr const char * NodeType_ = #CLASS_NAME; \
        public:\
    CLASS_NAME##Node(Node*left_node,Node * right_node) : BinaryNode(left_node,right_node){\
    } \
    virtual const char * get_node_type() const{\
        return NodeType_;\
    }\
}

BINARY_DEFINE(Mult);
BINARY_DEFINE(Div);
BINARY_DEFINE(Add);

class StoreNode: public Node{

    static constexpr const char * NodeType_ = "store"; 
    public:
    Node * addr_ptr_;
    Node * data_ptr_;

    StoreNode(Node * addr_ptr,  Node * data_ptr):addr_ptr_(addr_ptr),data_ptr_(data_ptr)
    {
        addr_ptr_->inc_need_node();
        data_ptr_->inc_need_node();
        type_ =  *addr_ptr_->get_type().get_pointer2type() ;
    }
    virtual const char * get_node_type() const{
        return NodeType_;
    }

};

#endif
