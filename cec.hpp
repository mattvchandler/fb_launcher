#ifndef CEC_HPP
#define CEC_HPP

#include "cectypes.h"
#include <functional>
#include <map>

#include <libcec/cec.h>

class CEC_Input
{
private:
    CEC::ICECAdapter * adapter_ {nullptr};
    CEC::libcec_configuration config_ {};
    CEC::ICECCallbacks callbacks_ {};

    static void keypress(void * cbparam, const CEC::cec_keypress * key);

    std::function<void(CEC::cec_user_control_code)> callback_ = [](CEC::cec_user_control_code){};

public:
    CEC_Input();
    ~CEC_Input();

    void register_callback(std::function<void(CEC::cec_user_control_code)> f) { callback_ = std::move(f); }

    void power_tv_on();

    operator bool() const { return adapter_; }
    operator CEC::ICECAdapter* () { return adapter_; }
    operator const CEC::ICECAdapter* () const { return adapter_; }
    CEC::ICECAdapter * operator->() { return adapter_; }
    const CEC::ICECAdapter * operator->() const { return adapter_; }
};

#endif // CEC_HPP
