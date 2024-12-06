//
// Created by malikt on 12/6/24.
//

#include "GeneralParameters.hpp"

#include <string>

GenPar* GenPar::instance_ = nullptr;

GenPar::GenPar()
{
    assert(instance_ == nullptr);
    instance_ = this;
};
void GenPar::Set(Params const param, int64_t const value)
{
    //void casting to bypass unused error as it is intentional
    assert(((void)"Attempted to access uninitialized singleon", (instance_ != nullptr)));
    instance_->config_[param]  = value;
}
int64_t GenPar::Get(Params param)
{
    assert(((void)"Attempted to access uninitialized singleon", (instance_ != nullptr)));
    assert(((void)"ERROR, uninitialized parameter attempted to be accessed", instance_->config_.contains(param)));
    return instance_->config_.at(param);
}
