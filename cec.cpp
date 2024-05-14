#include "cec.hpp"

#include <array>
#include <iostream>

CEC_Input::CEC_Input()
{
    const std::string devicename{"CECTest"};
    devicename.copy(config_.strDeviceName, std::min(devicename.size(), static_cast<std::string::size_type>(13)));

    config_.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
    config_.callbacks = &callbacks_;
    config_.callbackParam = this;
    callbacks_.keyPress = &CEC_Input::keypress;

    adapter_ = CECInitialise(&config_);
    if(!adapter_)
    {
        std::cerr << "Failed to initialize libcec" << std::endl;
        return;
    }

    std::array<CEC::cec_adapter_descriptor,10> devices;
    int8_t devices_found = adapter_->DetectAdapters(devices.data(), devices.size(), nullptr, true /*quickscan*/);
    if(devices_found <= 0)
    {
        std::cerr << "Could not automatically determine the CEC adapter device\n";
        CECDestroy(adapter_);
        adapter_ = nullptr;
        return;
    }

    // Open a connection to the zeroth CEC device
    if(!adapter_->Open(devices[0].strComName))
    {
        std::cerr << "Failed to open the CEC device on port " << devices[0].strComPath << std::endl;
        CECDestroy(adapter_);
        adapter_ = nullptr;
        return;
    }
    std::cout << "Opened the CEC device on port " << devices[0].strComPath << std::endl;
}

CEC_Input::~CEC_Input()
{
    if(adapter_)
    {
        adapter_->Close();
        CECDestroy(adapter_);
    }
}

void CEC_Input::keypress(void * cbparam, const CEC::cec_keypress * key)
{
    if(auto instance = static_cast<CEC_Input*>(cbparam); instance && key->duration == 0)
    {
        instance->callback_(key->keycode);
    }
}

void CEC_Input::power_tv_on()
{
    if(adapter_)
    {
        adapter_->PowerOnDevices(CEC::CECDEVICE_TV);
        adapter_->SetActiveSource();
    }
}
