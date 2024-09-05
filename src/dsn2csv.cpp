//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include <pugixml.hpp>

#include <dsn2csv.hpp>
#include <str-utils-static.hpp>
#include <file-utils-static.hpp>

std::string DSNTreeApp::Arguments(int argc, char* argv[])
{
	prog_path = std::filesystem::path(argv[0]).parent_path();
	prog_name = std::filesystem::path(argv[0]).filename();
	version = std::to_string(PROJECT_VERSION_MAJOR) + "." + std::to_string(PROJECT_VERSION_MINOR) + "." + std::to_string(PROJECT_VERSION_PATCH);
#ifdef PROJECT_VERSION_PRERELEASE
	if (!std::string(PROJECT_VERSION_PRERELEASE).empty()) {
		version += PROJECT_VERSION_PRERELEASE;
	}
#endif
	return ConsoleApp::Arguments(argc, argv);
}

void DSNTreeApp::SetUsage()
{
	us.description = prog_name.string() + " v" + version + " - Converts a DSN file to CSV, increasing its readibility.";
	us.set_syntax(prog_name.string() + " " + FILE_ARG + " [" + us.switch_char + "o:" + EXTENSION_ARG + "] [" + us.switch_char + "c:" + CONVERT_ARG
		+ "] [" + us.switch_char + "x:" + XMLDOC_ARG + "] [" + us.switch_char + "t] [" + us.switch_char + "v]");
	
	Unnamed_Arg file{ FILE_ARG };
	file.many = true;
	file.set_required(true);
	file.helpstring = "Filename(s) to process.";
	us.add_Argument(file);
	
	Named_Arg o{ EXTENSION_ARG };
	o.shortcut_char = 'o';
	o.set_type(Argument_Type::string);
	o.set_default_value(extension);
	o.helpstring = "Extension of the output file(s).";
	us.add_Argument(o);

	Named_Arg x{ XMLDOC_ARG };
	x.shortcut_char = 'x';
	x.set_type(Argument_Type::string);
	x.set_default_value(dsndesc_fname);
	x.helpstring = "XML document describing the DSN structure.";
	us.add_Argument(x);

	Named_Arg c{ CONVERT_ARG };
	c.shortcut_char = 'c';
	c.set_type(Argument_Type::string);
	std::string def{ "" };
	def.push_back(decimal_sep);
	c.set_default_value(def);
	c.helpstring = "Target decimal separator used for conversion.";
	us.add_Argument(c);

	Named_Arg t{ TRANSPOSE_ARG };
	t.shortcut_char = 't';
	t.set_type(Argument_Type::simple);
	t.helpstring = "Transpose wage types in columns.";
	us.add_Argument(t);

	Named_Arg v{ VERSION_ARG };
	v.shortcut_char = 'v';
	v.set_type(Argument_Type::simple);
	v.helpstring =	"Check that the DSN version of file matches the one given\n"
					"in the DSN description XML file.";
	us.add_Argument(v);
	
	us.usage = "This utility builds a file that extends the given source DSN file(s) for better\n"
				"readability / processing.\n"
				"The XML document describes the dependencies between blocks that can be present\n"
				"in a DSN file. See DSNTree.xml that is provided with this utility and used by\n"
				"default.\n"
				"If the convdec option is set, the dot '.' of decimal values is replaced with\n"
				"the given decimal separator and no string typing is applied.\n"
				"All levels of parent blocks are inserted before each block present in the input\n"
				"file. An ID is also provided for each set of common wage types, based on a\n"
				"decreasing wage type number within the same block.\n"
				"If the option transpose is used, the values of the wage types of each ID are\n"
				"printed on an single row.\n"
				"If the option version is used, the conversion is only performed if the DSN\n"
				"file version matches the version of the XML description file.\n";
}

std::string DSNTreeApp::CheckArguments()
{
	std::string retmsg{ "" };

	auto ext = us.get_Argument(EXTENSION_ARG);
	if (!ext->value.empty() && !ext->value.front().empty())
	{
		extension = ext->value.front();
		if (extension[0] != '.')
			extension.insert(0, ".");
	}

	auto xml = us.get_Argument(XMLDOC_ARG);
	if (!xml->value.empty() && !xml->value.front().empty())
		dsndesc_fname = xml->value.front();
	try {
		dsndesc_path = dsndesc_fname; }
	catch (const std::exception&) {
		retmsg = "Filename " + dsndesc_fname + " is not valid.";
		return retmsg; }
	if (!std::filesystem::exists(dsndesc_path))
	{
		if (dsndesc_fname.find('\\') == std::string::npos)
			dsndesc_path = prog_path.string() + "\\" + dsndesc_fname;		// try to find it in the program folder
		if (!std::filesystem::exists(dsndesc_path))
		{
			retmsg = "Filename " + dsndesc_fname + " does not exist.";
			return retmsg;
		}
	}

	auto dec = us.get_Argument(CONVERT_ARG);
	if (!dec->value.empty() && !dec->value.front().empty())
	{
		if (dec->value.front().length() > 1)
		{
			retmsg = "Decimal separator for conversion must be one character length.";
			return retmsg;
		}
		decimal_sep = dec->value.front().front();
	}

	auto trans = us.get_Argument(TRANSPOSE_ARG);
	if (!trans->value.empty() && trans->value.front() == "true")
		transpose = true;

	auto vers = us.get_Argument(VERSION_ARG);
	if (!vers->value.empty() && vers->value.front() == "true")
		version_check = true;

	return retmsg;				// all is okay
}

std::size_t DSNTreeApp::blockIndex(const std::string& id)
{
	auto itr = block_index.find(id);
	if (itr == block_index.end())
		return block_list.size();
	return (*itr).second;
}

DSNTreeApp::block& DSNTreeApp::getBlock(const std::string& id)
{
	auto itr = blockIndex(id);
	if (itr == block_list.size())
		return BAD_BLOCK;
	return block_list[itr];
}

DSNTreeApp::block& DSNTreeApp::getParent(const block& bl)
{
	auto child = blockIndex(bl.id);
	if (child == block_list.size())
		return BAD_BLOCK;
	auto itr = block_hie.find(child);
	if (itr == block_hie.end())
		return BAD_BLOCK;
	return block_list[(*itr).second];
}

// check if the string value represents a numeric value
// if _signed is true then the presence of a sign '+' or '-' is allowed at begin or end of the string, else no sign is allowed
bool isNumeric(const std::string& str, const std::string& fmtchars = ".", bool _signed = true)
{
	static const std::string NUMBERS{ "0123456789" };

	auto trimed = trimc(str);
	if (trimed.length() == 0)
		return false;
	std::string extNum{ NUMBERS + fmtchars };
	if (_signed)
	{
		extNum += "-+";
		if (trimed.front() != '-' && trimed.back() != '-' && trimed.find('-') != std::string::npos)
			return false;
		if (trimed.front() != '+' && trimed.back() != '+' && trimed.find('+') != std::string::npos)
			return false;
	}
	auto itr = trimed.find_first_not_of(extNum);
	if (itr != std::string::npos)
		return false;
	return true;
}

bool checkBlock(const std::string& blstr)
{
	static const int BLOCK_LENGTH{ 10 };

	if (blstr.length() != BLOCK_LENGTH)
		return false;
	if (blstr[0] != 'S' && blstr[0] != 's')
		return false;
	if (blstr[4] != 'G' && blstr[4] != 'g')
		return false;
	if (blstr[3] != '.' || blstr[7] != '.')
		return false;
	int i{ 1 };
	while (i < BLOCK_LENGTH)
	{
		if (blstr[i] < '0' || blstr[i] > '9')
			return false;
		i++;
		if (i == 3)
			i += 2;
		if (i == 7)
			i++;
	}
	return true;
}

std::string getBlockName(const std::string& sigstr)
{
	auto itr = sigstr.rfind('.');
	if (itr == std::string::npos)
		return "";
	return sigstr.substr(0, itr);
}

bool checkSignal(const std::string& sigstr)
{
	static const int WT_LENGTH{ 3 };

	auto lens = sigstr.length();
	auto block = getBlockName(sigstr);
	auto lenb = block.length();
	if (lenb == 0 || (lens - lenb - 1) != WT_LENGTH)
		return false;
	auto wt = sigstr.substr(lenb + 1, lens - lenb - 1);
	for (size_t i = 0; i < WT_LENGTH; i++)
		if (wt[i] < '0' || wt[i] > '9')
			return false;
	return checkBlock(block);
}

struct dsn_walker : pugi::xml_tree_walker
{
	const std::string ROOT_NODE{ "dsn_properties" };
	const std::string VERSION_NODE{ "version" };
	const std::string BLOCK_NODE{ "block" };
	const std::string ID_ATTRIBUTE{ "id" };
	const std::string NAME_NODE{ "name" };
	const std::string KEY_NODE{ "key" };

	DSNTreeApp &dsn_app;
	dsn_walker(DSNTreeApp& app) : pugi::xml_tree_walker(), dsn_app(app) {}

	bool parse_error{ false };					// is set to true if an error was detected during browse
	std::string error_msg{ "" };				// msg of the error found

	virtual bool begin(pugi::xml_node& node)
	{
		parse_error = false;
		error_msg = "";
		dsn_app.dsn_version = "";
		return true;
	}

	virtual bool for_each(pugi::xml_node& node)
	{
		if (node.name() == VERSION_NODE)
			if (node.first_child() != nullptr && node.first_child().value() != nullptr)
				dsn_app.dsn_version = node.first_child().value();
		if (node.name() == BLOCK_NODE)
		{
			if (auto d = depth(); d > dsn_app.dsntree_depth)
				dsn_app.dsntree_depth = d;
			if (node.first_attribute() != nullptr && node.first_attribute().name() == ID_ATTRIBUTE)
			{
				DSNTreeApp::block bl;
				bl.id = node.first_attribute().value();
				if (!checkBlock(bl.id))
				{
					parse_error = true;
					error_msg = "Block id " + bl.id + " is not valid";
					return false;
				}
				bl.name = "";
				bl.key = "001";
				if (dsn_app.blockIndex(bl.id) == dsn_app.block_list.size())
				{
					auto blindex = dsn_app.block_list.size();
					dsn_app.block_list.push_back(bl);
					dsn_app.block_index[bl.id] = blindex;
					auto parent = node.parent();
					if (parent != nullptr && parent.name() == BLOCK_NODE)
					{
						assert((parent.first_attribute() != nullptr && parent.first_attribute().name() == ID_ATTRIBUTE)
							&& "A parent block without id attribute has passed the check");		// this should never happen
						auto pbid = dsn_app.blockIndex(parent.first_attribute().value());
						assert((pbid != dsn_app.block_list.size()) && "A parent block has not yet been created");	// this should never happen
						dsn_app.block_hie[blindex] = pbid;
					}
					else
					{
						if (parent != nullptr && parent.name() == ROOT_NODE)
							dsn_app.block_hie[blindex] = 0;
						else
						{
							parse_error = true;
							error_msg = "Block " + bl.id + " is out of the dsn_properties node scope";
							return false;
						}
					}
				}
				else
				{
					parse_error = true;
					error_msg = "Block " + bl.id + " is defined more than one time";
					return false;
				}
			}
			else
			{
				parse_error = true;
				error_msg = "Block without id attribute";
				return false;
			}
		}
		if (node.name() == NAME_NODE || node.name() == KEY_NODE)
		{
			if (node.first_child() != nullptr)
			{
				auto parent = node.parent();
				if (parent != nullptr && parent.name() == BLOCK_NODE)
				{
					assert((parent.first_attribute() != nullptr && parent.first_attribute().name() == ID_ATTRIBUTE)
						&& "A parent block without id attribute has passed the check");		// this should never happen
					auto pbid = dsn_app.blockIndex(parent.first_attribute().value());
					assert((pbid != dsn_app.block_list.size()) && "A parent block has not yet been created");	// this should never happen
					if (node.name() == NAME_NODE)
						dsn_app.block_list[pbid].name = node.first_child().value();
					else
						dsn_app.block_list[pbid].key = node.first_child().value();
				}
				else
				{
					parse_error = true;
					error_msg = node.name();			// insert it in 2 times because pugi::char_t is not fully compatible with string
					error_msg = "A " + error_msg + " node is defined out of the scope of a block";
					return false;
				}
			}
		}
		return true;		// continue to browse the nodes
	}
};

void DSNTreeApp::PreProcess()
{
	pugi::xml_parse_result result = dsndesc_doc.load_file(dsndesc_path.c_str());
	if (!result)
		throw std::filesystem::filesystem_error("Error at parsing " + dsndesc_path.string() + ": " + result.description(), std::make_error_code(std::errc::operation_canceled));
	// browse the DSN tree to determine the max number of parents
	dsn_walker walker(*this);
	dsndesc_doc.traverse(walker);
	if (walker.parse_error)
		throw std::filesystem::filesystem_error("Error in DSN descriptor XML file " + dsndesc_path.string() + ": " + walker.error_msg,
			std::make_error_code(std::errc::operation_not_supported));
	std::cout << "File " << dsndesc_path.filename() << ": DSN blocks description is version " << dsn_version << "." << std::endl;
}

std::string formatNumber(const unsigned long num, const size_t len)
{
	std::string ret = std::to_string(num);
	while (ret.length() < len)
		ret = '0' + ret;
	return ret;
}

std::string substc(const std::string& str, const char _old, const char _new)
{
	std::string res{ str };

	auto itr = res.find(_old);
	while (itr != std::string::npos)
	{
		res[itr] = _new;
		itr = res.find(_old);
	}
	return res;
}

void DSNTreeApp::MainProcess(const std::filesystem::path& file)
{
#ifdef _WIN32
	static const std::string CMD_LINE{ "copy /Y " };
#elif __unix__
	static const std::string CMD_LINE{ "cp " };
#endif
	static const unsigned long long DEFAULT_INCREMENT = 1000;
	static const unsigned long long AVER_ROW_LEN = 25;
	static const char DSN_SEPARATOR = ',';
	static const unsigned long long MAX_LINES_TO_VERSION = 20;
	static const std::string VERSION_SIGNAL = "S10.G00.00.006";
	static const std::string SEQ_KEY = "seq";
	static const size_t SEQ_LENGTH = 5;

	// initialize input file
	auto fsize = std::filesystem::file_size(file);
	auto EOL_type = file_EOL(file);
	auto EOL_len = EOL_length(EOL_type);
	char EOL_delim = '\n';
	if (EOL_type == EOL::Mac)
		EOL_delim = '\r';
	std::ifstream infile(file, std::ios::binary);
	std::uintmax_t lineCnt{ 0 };
	
	// initialize output file
	auto outpath = getOutPath(file);
	auto outpath_tmp = outpath;
	outpath_tmp.replace_extension(".body.tmp");
	if (std::filesystem::exists(outpath))
		std::filesystem::remove(outpath);
	if (std::filesystem::exists(outpath_tmp))
		std::filesystem::remove(outpath_tmp);
	if (transpose)
		outpath.swap(outpath_tmp);
	std::ofstream outfile(outpath, std::ios_base::out | std::ios::binary);
	std::uintmax_t outCnt{ 0 };

	// initialize tmp header file
	std::string tmpname{ file.generic_string() };
	tmpname.append(".head.tmp");
	std::filesystem::path tmppath(tmpname);
	if (std::filesystem::exists(tmppath))
		std::filesystem::remove(tmppath);
	std::ofstream tmpfile(tmppath, std::ios_base::out);

	std::ostringstream header;
	unsigned int headlen{ 0 };
	for (size_t i = 0; i < dsntree_depth - 1; i++)
	{
		if (i != 0)
			header << ';';
		header << "Key" << i << ".Id;Key" << i << ".Val";
		headlen += 2;
	}
	header << ";Block;Name;Seq;";
	headlen += 3;

	// transposition needs to write the header after processing entire file
	if (!transpose)
	{
		header << "WT;Value";
		outfile << header.str() << "\n";
		outCnt++;
	}

	// print DSN version and check if it matches the DSN description version present in XML file, if requested
	std::string buf;
	bool cont{ true };
	auto siglen = VERSION_SIGNAL.length();
	while (infile && cont)
	{
		std::getline(infile, buf, EOL_delim);
		lineCnt++;
		auto buflen = buf.length();
		bool errfmt{ false };
		bool bad_version{ false };
		if (buflen <= siglen)
			errfmt = true;
		else
			if (buf.substr(0, siglen) == VERSION_SIGNAL)
			{
				if (EOL_type == EOL::Windows)
					if (buf.back() == '\r')
						buf.pop_back();
				auto itr = buf.find(',');
				if (itr == std::string::npos)
					errfmt = true;
				else
				{
					auto value = buf.substr(itr + 1, buflen - itr - 1);
					if (value.front() != '\'' || value.back() != '\'')
						errfmt = true;
					else
					{
						value.pop_back();
						value.erase(0, 1);
						std::cout << "DSN version of file " << file.filename() << " is " << value << "." << std::endl;
						cont = false;
						if (version_check && dsn_version != value)
						{
							std::cout << "DSN version mismatch, file aborted." << std::endl;
							bad_version = true;
						}
					}
				}
			}
		if (errfmt)
			std::cout << "Unrecognized format '" << buf << "' at line " << lineCnt << ", file aborted." << std::endl;
		if (lineCnt > MAX_LINES_TO_VERSION)		// version should be present at line 6
		{
			std::cout << "DSN version key '" << VERSION_SIGNAL << "' not found while this is obligatory, file aborted." << std::endl;
			errfmt = true;
		}
		if (errfmt || bad_version)
		{
			infile.close();
			outfile.close();
			tmpfile.close();
			std::filesystem::remove(outpath);
			std::filesystem::remove(tmppath);
			return;
		}
	}
	infile.seekg(0);
	lineCnt = 0;

	long long currPos = infile.tellg();
	unsigned long long increment;
	if ((increment = ((fsize / AVER_ROW_LEN / 100) / DEFAULT_INCREMENT) * DEFAULT_INCREMENT) < DEFAULT_INCREMENT)
		increment = DEFAULT_INCREMENT;
	std::string prevBlock;
	size_t blIndex = block_list.size();
	size_t noBlock = blIndex;
	std::vector<size_t> blockHeap;		// to store previous seq while processing childs
	size_t prevIndex = blIndex;
	std::string parents;
	unsigned long seq{ 0 };
	unsigned int prevwt{ 0 };
	unsigned int headwt{ 1 };
	unsigned int wtcount{ 0 };
	while (infile)
	{
		std::getline(infile, buf, EOL_delim);
		lineCnt++;
		auto buflen = buf.length();
		if (buflen != 0)
		{
			if (EOL_type == EOL::Windows)
				if (buf.back() == '\r')
					buf.pop_back();
			bool errfmt{ false };
			std::string signal;
			std::string value;
			auto sep = buf.find(',');
			if (sep == std::string::npos)
				errfmt = true;
			else
			{
				signal = buf.substr(0, sep);
				value = buf.substr(sep + 1, buflen - sep - 1);
				if (!checkSignal(signal) || value.front() != '\'' || value.back() != '\'')
					errfmt = true;
			}
			std::string blname;
			if (errfmt)
				std::cout << std::endl << "Unrecognized format '" << buf << "' at line " << lineCnt << ", file aborted." << std::endl;
			else
			{
				blname = getBlockName(signal);
				if (blname != prevBlock)
					blIndex = blockIndex(blname);
				if (blIndex == noBlock)
				{
					std::cout << std::endl << "Unknown block identifier '" << blname << "' at line " << lineCnt << ", file aborted." << std::endl;
					errfmt = true;
				}
			}
			if (errfmt)
			{
				tmpfile.close();
				infile.close();
				outfile.close();
				std::filesystem::remove(outpath);
				std::filesystem::remove(tmppath);
				return;
			}
			bool assign_seqkey{ false };
			if (blname != prevBlock)
			{
				// build parents string
				parents.clear();
				auto p = block_hie[blIndex];
				int pcount{ 0 };
				while (p != 0)
				{
					parents = block_list[p].id + "." + block_list[p].key + ";=\"" + block_list[p].lastValue + "\";" + parents;
					pcount++;
					if (p == prevIndex)							// stores the seq of the parent while processing childs
					{
						blockHeap.push_back(p);
						block_list[p].lastSeq = seq;
					}
					p = block_hie[p];
				}
				while (pcount < dsntree_depth - 1)
				{
					parents = parents + ";;";
					pcount++;
				}
				if (transpose && prevwt != 0)
				{
					outfile << '\n';
					outCnt++;
				}
				prevwt = 0;
				seq = 0;
				auto itr = std::find(blockHeap.begin(), blockHeap.end(), blIndex);
				if (itr != blockHeap.end())
				{
					seq = block_list[blIndex].lastSeq;			// restore the saved value of seq
					while (blockHeap.back() != blIndex)
						blockHeap.pop_back();					// deletes the childs of the element from the heap
					blockHeap.pop_back();
					seq++;
				}
				wtcount = 0;
				if (block_list[blIndex].key == SEQ_KEY)
					assign_seqkey = true;
			}
			value.pop_back();
			value.erase(0, 1);
			auto wt = signal.substr(blname.size() + 1, 3);
			auto wtnum = std::stoul(wt);
			if (wtnum < prevwt)
			{
				seq++;
				wtcount = 0;
				if (block_list[blIndex].key == SEQ_KEY)
					assign_seqkey = true;
				if (transpose)
				{
					outfile << '\n';
					outCnt++;
				}
			}
			if (assign_seqkey)
				block_list[blIndex].lastValue = formatNumber(seq, SEQ_LENGTH);
			wtcount++;
			if (wtcount > headwt)
				headwt++;
			if (block_list[blIndex].key == wt)
				block_list[blIndex].lastValue = value;
			if (!transpose || wtcount == 1)
				outfile << parents << blname << ';' << block_list[blIndex].name << ";=\"" << formatNumber(seq, SEQ_LENGTH) << "\";";
			if (transpose && wtcount > 1)
				outfile << ';';
			outfile << "=\"" << wt << "\";";
			if (decimal_sep == ' ' || value.find('.') == std::string::npos || !isNumeric(value))
				outfile << "=\"" << value << "\"";
			else
				outfile << substc(value, '.', decimal_sep);
			if (!transpose)
			{
				outfile << '\n';
				outCnt++;
			}
			prevwt = wtnum;
			prevBlock = blname;
			prevIndex = blIndex;
		}
		if ((currPos = infile.tellg()) == -1)
			currPos = fsize;
		if (lineCnt % increment == 0 || !infile)
			std::cout << "\rReading " << file.filename() << " : " << lineCnt << " lines (" << currPos * 100 / fsize << "%)";
	}

	std::cout << std::endl;
	infile.close();

	if (transpose)
	{
		outfile << '\n';
		outCnt++;
	}
	outfile.close();

	if (transpose)
	{
		for (size_t i = 0; i < headwt; i++)
		{
			if (i != 0)
				header << ';';
			header << "WT" << i << ";Val" << i;
		}
		tmpfile << header.str() << std::endl;
	}
	tmpfile.close();
	if (transpose)
	{
		outpath.swap(outpath_tmp);
		tmppath.make_preferred();
		outpath.make_preferred();
		outpath_tmp.make_preferred();
		std::string command{ CMD_LINE + tmppath.string() + " + " + outpath_tmp.string() + " " + outpath.string() + " > nul"};
		auto ret = std::system(command.c_str());
		outCnt++;
		std::filesystem::remove(outpath_tmp);
	}
	std::filesystem::remove(tmppath);

	std::cout << "File " << outpath.filename() << ": " << std::to_string(outCnt) << " lines written.\n" << std::endl;
}
