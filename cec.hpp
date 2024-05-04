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

    std::map<CEC::cec_user_control_code, std::function<void()>> callback_directory_;

public:
    CEC_Input();
    ~CEC_Input();

    void register_up(std::function<void()> f) { callback_directory_[CEC::CEC_USER_CONTROL_CODE_UP] = std::move(f); }
    void register_down(std::function<void()> f) { callback_directory_[CEC::CEC_USER_CONTROL_CODE_DOWN] = std::move(f); }
    void register_left(std::function<void()> f) { callback_directory_[CEC::CEC_USER_CONTROL_CODE_LEFT] = std::move(f); }
    void register_right(std::function<void()> f) { callback_directory_[CEC::CEC_USER_CONTROL_CODE_RIGHT] = std::move(f); }
    void register_select(std::function<void()> f) { callback_directory_[CEC::CEC_USER_CONTROL_CODE_SELECT] = std::move(f); }
    void register_EXIT(std::function<void()> f) { callback_directory_[CEC::CEC_USER_CONTROL_CODE_EXIT] = std::move(f); }

    void register_by_code(CEC::cec_user_control_code code, std::function<void()> f) { callback_directory_[code] = std::move(f); }

    operator bool() const { return adapter_; }
    operator CEC::ICECAdapter* () { return adapter_; }
    operator const CEC::ICECAdapter* () const { return adapter_; }
    CEC::ICECAdapter * operator->() { return adapter_; }
    const CEC::ICECAdapter * operator->() const { return adapter_; }
};

#endif // CEC_HPP
