#pragma once

#include <map>

#include <consoleapp-static.hpp>
//#include <utils/utils.hpp>

#include <pugixml.hpp>

struct dsn_walker;

class DSNTreeApp : public ConsoleApp
{
	friend struct dsn_walker;

public:
#ifdef _WIN32
	DSNTreeApp(bool window_mode) : ConsoleApp(window_mode) {};
#else
	DSNTreeApp() : ConsoleApp() {};
#endif

	std::string Arguments(int argc, char* argv[]);

	// final parameters storage
	std::filesystem::path prog_path;
	std::string extension{ ".tree.csv" };
	std::string dsndesc_fname{ "DSNTree.xml" };
	char decimal_sep{ ' ' };
	std::filesystem::path dsndesc_path;
	pugi::xml_document dsndesc_doc;
	std::string dsn_version;
	int dsntree_depth{ 0 };
	bool transpose{ false };
	bool version_check{ false };

	// argument names of the application
	const std::string FILE_ARG{ "file" };
	const std::string EXTENSION_ARG{ "extension" };
	const std::string XMLDOC_ARG{ "xmldocument" };
	const std::string CONVERT_ARG{ "convdec" };
	const std::string TRANSPOSE_ARG{ "transpose" };
	const std::string VERSION_ARG{ "version" };

protected:
	virtual void SetUsage() override;											// Defines expected arguments and help.
	virtual std::string CheckArguments() override;								// Performs more accurate checks and initializations if necessary
	virtual void PreProcess() override;
	virtual void MainProcess(const std::filesystem::path& file) override;		// Launched by ByFile for each file matching argument 'file' values

private:
	struct block
	{
		std::string id;
		std::string name;
		std::string key;
		std::string lastValue;
		unsigned long lastSeq;
		friend bool operator ==(const block& b1, const block& b2) { return b1.id == b2.id; }
		friend bool operator !=(const block& b1, const block& b2) { return b1.id != b2.id; }
	};
	block BAD_BLOCK{ "" };								// used to handle errors
	block ROOT_BLOCK{ "root" };							// used to define the parent of root blocks
	std::vector<block> block_list{ ROOT_BLOCK };		// the ROOT block is the 1st item of the list
	std::map<std::string, size_t> block_index;			// used to search block by id
	std::map<size_t, size_t> block_hie;					// used to link parent to each block

	size_t blockIndex(const std::string& id);
	block& getBlock(const std::string& id);
	block& getParent(const block& bl);
};
