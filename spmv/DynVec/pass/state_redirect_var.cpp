#include "state_formulation.hpp"
#include "statement_print.hpp"
#include "state_pass.hpp"
class RedirectVarPass : public StateMentPass{
    private:
    std::map<Varience*,Varience*> change_var_var_;
    public: 

    virtual    StateMent* pass_(LetStat * stat ) ;

    virtual    StateMent* pass_(Varience * stat  ) ;
};
StateMent * RedirectVarPass::pass_(Varience * stat) {
    auto change_var_var_it = change_var_var_.find( stat );
    if(change_var_var_it == change_var_var_.end()) {
        return stat;
    } else {
        return change_var_var_it->second; 
    }
}
StateMent * RedirectVarPass::pass_(LetStat * stat) {
    
    Varience * res_var = stat->get_res();
    StateMent * state_expr = stat->get_expr();
    StateMent * state_expr_new = pass(state_expr);
    Varience * res_var_new = res_var;

    if( res_var->get_is_const()   ) {
        res_var_new = new Varience( state_expr_new->get_type(), res_var->node_name_ );
        res_var_new->index_name_ = res_var->index_name_;
        res_var_new->addr_name_ = res_var->addr_name_;
        change_var_var_[ res_var ] = res_var_new;
    }
    if(res_var_new == res_var && state_expr == state_expr_new ) {
        return stat;
    } else {
        return LetStat::make(res_var_new,state_expr_new); 
    }
}
StateMent * redirect_var_state( StateMent * state ) {
    RedirectVarPass redirect_pass;
    
    //StateMentPass redirect_pass;
    StateMent * form_state =  redirect_pass.pass(state);

    return form_state;
}
