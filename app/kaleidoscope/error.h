#pragma once
#include <system_error>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <comdef.h>
#include <d3d12.h>

namespace must
{

auto constexpr done = [](std::convertible_to<BOOL> auto && value)
{
    if (static_cast<bool>(value))
        return std::forward<decltype(value)>(value);

    auto code = ::GetLastError();
    auto desc = std::system_category().message(code);
    ::OutputDebugString(desc.c_str());

    // Note: to use OutputDebugString in our cmake target,
    // we may need to add the following env to "launch.vs.json" file
    //
    // ```json
    // "env": {
    //     "DEBUG_LOGGING_LEVEL": "trace;info",
    //     "ENABLE_TRACING" : "true"
    // }
    // ```
    //
    // see:
    // https://learn.microsoft.com/en-us/cpp/build/configure-cmake-debugging-sessions?view=msvc-170#launchvsjson-reference

    throw std::system_error(code, std::system_category());
};

auto constexpr non_null = []<typename T>(T * value)
{
    done(value != nullptr);
    return value;
};

auto constexpr succeed = [](HRESULT hr)
{
    if (SUCCEEDED(hr))
        return hr;

    // About error message:
    // https://stackoverflow.com/a/7008111
    auto err  = _com_error(hr);
    auto desc = err.ErrorMessage();
    OutputDebugString(desc);

    throw err;
};

auto constexpr succeed_with = [](ID3DBlob & message)
{
    return [&](HRESULT hr)
    {
        if (SUCCEEDED(hr))
            return hr;

        OutputDebugString(static_cast<char const *>(message.GetBufferPointer()));
        throw _com_error(hr);
    };
};
} // namespace must
