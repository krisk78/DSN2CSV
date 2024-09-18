#pragma once

#include <map>
#include <unordered_map>

#include <xlnt/xlnt.hpp>

#include <consoleapp-static.hpp>
#include <file-utils-static.hpp>
#include <configuration.hpp>

//#include <pugixml.hpp>

//struct dsn_walker;

class ApplicationException : public std::exception {
public:
	explicit ApplicationException(const std::string& message)
		: message_(message) {}

	virtual const char* what() const noexcept override {
		return message_.c_str();
	}

private:
	std::string message_;
};

class DSNTreeApp : public ConsoleApp::ConsoleApp
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
	std::filesystem::path prog_name;
	std::string version;
	bool csv{ false };
	std::string extension{ ".csv" };
	bool xlsx{ false };
	//std::string dsndesc_fname{ "DSN.xml" };
	std::string dsndesc_fname;
	std::filesystem::path dsndesc_path;
	bool use_cat_texts{ false };
	char decimal_sep{ ' ' };
	//pugi::xml_document dsndesc_doc;
	std::string dsn_version;
	size_t dsntree_depth{ 0 };
	bool transpose{ false };
	bool version_check{ false };

	// argument names of the application
	const std::string FILE_ARG{ "file" };
	const std::string CSV_ARG{ "csv" };
	const std::string EXTENSION_ARG{ "extension" };
	const std::string XLSX_ARG{ "xlsx" };
	//const std::string XMLDOC_ARG{ "xmldocument" };
	const std::string DSN_REF_ARG{ "dictionary" };
	const std::string CATEGORY_ARG{ "category" };
	const std::string CONVERT_ARG{ "convdec" };
	const std::string TRANSPOSE_ARG{ "transpose" };
	const std::string VERSION_ARG{ "version" };

protected:
	virtual void SetUsage() override;											// Defines expected arguments and help.
	virtual std::string CheckArguments() override;								// Performs more accurate checks and initializations if necessary
	virtual void PreProcess() override;
	virtual void MainProcess(const std::filesystem::path& file) override;		// Launched by ByFile for each file matching argument 'file' values

private:

	Configuration config;

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

	std::map<std::string, std::map<std::string, std::string>> cat_texts;			// used to retrieve category text by block id and category id
	const std::string& getCategoryText(const std::string& block_id, const std::string& category_id);

	size_t blockIndex(const std::string& id);
	block& getBlock(const std::string& id);
	block& getParent(const block& bl);

	std::string getKey(const std::string& id) const;
	void addBlock(const std::string& id, const size_t parent);

	std::unordered_map<size_t, size_t> depth_cache;
	size_t getDepth(size_t node);

	void readRootNodes(xlnt::workbook& wb);
	void readBlocks(xlnt::workbook& wb);
	void readFields(xlnt::workbook& wb);
	void getDSNDescription();

	bool checkVersion(const std::filesystem::path& filename, std::ifstream& file, const char EOL_delim, const file_utils::EOL EOL_type) const;

	std::string buildParentString(size_t blIndex, size_t prevIndex, long seq, std::vector<size_t>& blockHeap);
	bool processFile(const std::filesystem::path& filename, std::ifstream& infile, const std::uintmax_t fsize, const char EOL_delim, const file_utils::EOL EOL_type,
		std::ofstream& outfile, std::uintmax_t& outCnt, unsigned int& headwt);

};
