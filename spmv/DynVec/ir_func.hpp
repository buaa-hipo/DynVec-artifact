#ifndef IR_FUNC_HPP
#define IR_FUNC_HPP
#include <string>
#include <iostream>
#include <vector>
#include <functional>
template<typename FType>
class ir_func;
#define SET_DISPATCH_TEMPLATE(IR_FUNC_PTR,CLASS_NAME,BASIC_CLASS_NAME,FUNCTION) \
            IR_FUNC_PTR->set_dispatch<CLASS_NAME>([this](BASIC_CLASS_NAME * __basic_class_ptr,std::ostream &os){\
                CLASS_NAME * __class_name = dynamic_cast<CLASS_NAME*>( __basic_class_ptr );\
                if(__class_name==NULL){printf("type "#BASIC_CLASS_NAME "-> "#CLASS_NAME"failt");exit(1);}\
                this->FUNCTION(__class_name, os);})\

template<typename BaseClassPtr,typename R,typename ...Args>
class ir_func<R(BaseClassPtr,Args...)> {

    using Function = std::function<R(BaseClassPtr,Args...)>;
    using TSelf = ir_func<R(BaseClassPtr,Args...)>;
    std::vector<Function> funcs_;
    public:
    template<typename ClassType>
    TSelf & set_dispatch(Function f) {
        const int tindex = GetTypeId(ClassType::class_name_);
        if( funcs_.size() <= tindex ) {
            funcs_.resize(tindex+1,nullptr);
        }
        if( funcs_[tindex] != nullptr) {
            printf("Dispatch for %d is already set",tindex);
            exit(1);
        }
        funcs_[tindex] = f;
        return *this;
    }
    inline R operator() (BaseClassPtr base_ptr, Args ...args ) {
        //int type_index = base_ptr->get_typeid();
        int type_index = GetTypeId( base_ptr->get_class_name());
        if( type_index >= funcs_.size() || funcs_[type_index] == nullptr) {
            LOG(FATAL) << "can not find " <<  base_ptr->get_class_name()<<"\n";
            exit(1);
        }
        return funcs_[type_index](base_ptr,std::forward<Args>(args)...);
    }
};
#endif

