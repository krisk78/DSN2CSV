#include <configuration.hpp>

void Configuration::readConfig(const std::filesystem::path& filename)
{
    libconfig::Config cfg;

    // Read the file. If there is an error, report it and exit.
    try
    {
        cfg.readFile(filename.string());
    }
    catch (const libconfig::FileIOException& fioex)
    {
        throw ConfigurationException("I/O error while reading file " + filename.string() + ": " + fioex.what() + ".");
    }
    catch (const libconfig::ParseException& pex)
    {
        throw ConfigurationException("Parse error at " + filename.string() + ", line " + std::to_string(pex.getLine()) + ": " + pex.getError() + ".");
    }

    // Access and set the configuration values using TOSTRING macro
    if (!cfg.lookupValue(TOSTRING(version.worksheet), version.worksheet))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(version.worksheet) + ".");
    if (!cfg.lookupValue(TOSTRING(version.cell), version.cell))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(version.cell) + ".");
    if (!cfg.lookupValue(TOSTRING(version.signal), version.signal))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(version.signal) + ".");
    if (!cfg.lookupValue(TOSTRING(header.worksheet), header.worksheet))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(header.worksheet) + ".");
    if (!cfg.lookupValue(TOSTRING(header.cell), header.cell))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(header.cell) + ".");
    if (!cfg.lookupValue(TOSTRING(blocks.worksheet), blocks.worksheet))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(blocks.worksheet) + ".");
    if (!cfg.lookupValue(TOSTRING(blocks.cell), blocks.cell))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(blocks.cell) + ".");
    if (!cfg.lookupValue(TOSTRING(blocks.parent_column), blocks.parent_column))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(blocks.parent_column) + ".");
    if (!cfg.lookupValue(TOSTRING(fields.worksheet), fields.worksheet))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(fields.worksheet) + ".");
    if (!cfg.lookupValue(TOSTRING(fields.cell), fields.cell))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(fields.cell) + ".");
    if (!cfg.lookupValue(TOSTRING(fields.block_column), fields.block_column))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(fields.block_column) + ".");
    if (!cfg.lookupValue(TOSTRING(fields.descr_column), fields.descr_column))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(fields.descr_column) + ".");
    std::string buf;
    if (!cfg.lookupValue(TOSTRING(fields.text_delimiter), buf))
        throw ConfigurationException("Missing entry in configuration file " + filename.string() + ": " + TOSTRING(fields.text_delimiter) + ".");
    fields.text_delimiter = buf[0];

    // Access and set the missing parents
    const libconfig::Setting& missingSetting = cfg.lookup("missing");
    for (int i = 0; i < missingSetting.getLength(); ++i) {
        std::string block = missingSetting[i][0];
        std::string parent = missingSetting[i][1];
        missing[block] = parent;
    }

    // Access and set the special keys
    const libconfig::Setting& keysSetting = cfg.lookup("keys");
    for (int i = 0; i < keysSetting.getLength(); ++i)
    {
        std::string block = keysSetting[i][0];
        std::string key = keysSetting[i][1];
        keys[block] = key;
    }
}
