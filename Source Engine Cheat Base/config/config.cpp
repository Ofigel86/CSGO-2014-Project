#include "config.hpp"
#include <fstream>
#include <sstream>

c_config_manager* g_config_manager = new c_config_manager();
c_config_variables* g_variables = new c_config_variables();

void c_config_manager::Save(std::string path)
{
    if (path.empty() || !g_variables)
        return;

    __try {
        std::ofstream file(path);
        if (!file.is_open())
            return;

        // Simple JSON-like format
        file << "{\n";
        file << "  \"ragebot_enabled\": " << (g_variables->ragebot_enabled ? "true" : "false") << ",\n";
        file << "  \"ragebot_autofire\": " << (g_variables->ragebot_autofire ? "true" : "false") << ",\n";
        file << "  \"ragebot_autoscope\": " << (g_variables->ragebot_autoscope ? "true" : "false") << ",\n";
        file << "  \"legitbot_enabled\": " << (g_variables->legitbot_enabled ? "true" : "false") << ",\n";
        file << "  \"chams_enabled\": " << (g_variables->chams_enabled ? "true" : "false") << ",\n";
        file << "  \"esp_enabled\": " << (g_variables->esp_enabled ? "true" : "false") << ",\n";
        file << "  \"visuals_thirdperson\": " << (g_variables->visuals_thirdperson ? "true" : "false") << ",\n";
        file << "  \"movement_bunnyhop\": " << (g_variables->movement_bunnyhop ? "true" : "false") << ",\n";
        file << "  \"misc_nospread\": " << (g_variables->misc_nospread ? "true" : "false") << "\n";
        file << "}\n";

        file.close();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_config_manager::Load(std::string path)
{
    if (path.empty() || !g_variables)
        return;

    __try {
        std::ifstream file(path);
        if (!file.is_open())
            return;

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Very simple parser - just check if contains true
        auto parse_bool = [&](const std::string& key) -> bool {
            size_t pos = content.find("\"" + key + "\"");
            if (pos == std::string::npos) return false;
            size_t colon = content.find(":", pos);
            if (colon == std::string::npos) return false;
            size_t true_pos = content.find("true", colon);
            size_t false_pos = content.find("false", colon);
            if (true_pos != std::string::npos && (false_pos == std::string::npos || true_pos < false_pos))
                return true;
            return false;
        };

        g_variables->ragebot_enabled = parse_bool("ragebot_enabled");
        g_variables->ragebot_autofire = parse_bool("ragebot_autofire");
        g_variables->ragebot_autoscope = parse_bool("ragebot_autoscope");
        g_variables->legitbot_enabled = parse_bool("legitbot_enabled");
        g_variables->chams_enabled = parse_bool("chams_enabled");
        g_variables->esp_enabled = parse_bool("esp_enabled");
        g_variables->visuals_thirdperson = parse_bool("visuals_thirdperson");
        g_variables->movement_bunnyhop = parse_bool("movement_bunnyhop");
        g_variables->misc_nospread = parse_bool("misc_nospread");
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}
