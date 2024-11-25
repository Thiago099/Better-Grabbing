#pragma once

#include <spdlog/sinks/basic_file_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace logger = SKSE::log;
using namespace std::literals;

using FormID = RE::FormID;
using RefID = RE::FormID;

constexpr RefID player_refid = 20;

const bool is_grab_n_throw_installed = std::filesystem::exists("Data/SKSE/Plugins/po3_GrabAndThrow.dll");