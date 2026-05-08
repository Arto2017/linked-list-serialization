#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct ListNode {
	ListNode* prev = nullptr;
	ListNode* next = nullptr;
	ListNode* rand = nullptr;
	std::string data;
};

void freeList(ListNode* head) {
	while (head != nullptr) {
		ListNode* next = head->next;
		delete head;
		head = next;
	}
}

ListNode* buildListFromText(std::ifstream& input) {
	std::vector<std::string> allData;
	std::vector<int64_t> allRandIndexes;

	std::string line;
	while (std::getline(input, line)) {
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		if (line.empty()) {
			continue;
		}

		size_t separatorPos = line.rfind(';');
		if (separatorPos == std::string::npos) {
			continue;
		}

		std::string data = line.substr(0, separatorPos);
		std::string indexPart = line.substr(separatorPos + 1);

		int64_t randIndex = -1;
		try {
			randIndex = std::stoll(indexPart);
		} catch (...) {
			randIndex = -1;
		}

		allData.push_back(std::move(data));
		allRandIndexes.push_back(randIndex);
	}

	if (allData.empty()) {
		return nullptr;
	}

	std::vector<ListNode*> nodes;
	nodes.reserve(allData.size());

	for (size_t i = 0; i < allData.size(); ++i) {
		ListNode* node = new ListNode();
		node->data = std::move(allData[i]);
		nodes.push_back(node);
	}

	for (size_t i = 0; i < nodes.size(); ++i) {
		if (i > 0) {
			nodes[i]->prev = nodes[i - 1];
		}
		if (i + 1 < nodes.size()) {
			nodes[i]->next = nodes[i + 1];
		}

		int64_t randIndex = allRandIndexes[i];
		if (randIndex >= 0 && static_cast<size_t>(randIndex) < nodes.size()) {
			nodes[i]->rand = nodes[static_cast<size_t>(randIndex)];
		}
	}

	return nodes.front();
}

void serialize(ListNode* head, std::ofstream& out) {
	std::vector<ListNode*> nodes;
	std::unordered_map<ListNode*, size_t> indexOf;

	for (ListNode* cur = head; cur != nullptr; cur = cur->next) {
		indexOf.emplace(cur, nodes.size());
		nodes.push_back(cur);
	}

	uint64_t count = static_cast<uint64_t>(nodes.size());
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));

	for (ListNode* node : nodes) {
		uint32_t dataSize = static_cast<uint32_t>(node->data.size());
		out.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
		if (dataSize > 0) {
			out.write(node->data.data(), static_cast<std::streamsize>(dataSize));
		}

		int64_t randIndex = -1;
		if (node->rand != nullptr) {
			auto it = indexOf.find(node->rand);
			if (it != indexOf.end()) {
				randIndex = static_cast<int64_t>(it->second);
			}
		}
		out.write(reinterpret_cast<const char*>(&randIndex), sizeof(randIndex));
	}
}

ListNode* deserialize(std::ifstream& in) {
	uint64_t count = 0;
	in.read(reinterpret_cast<char*>(&count), sizeof(count));
	if (!in || count == 0) {
		return nullptr;
	}

	std::vector<ListNode*> nodes;
	nodes.reserve(static_cast<size_t>(count));
	std::vector<int64_t> randIndexes;
	randIndexes.reserve(static_cast<size_t>(count));

	for (uint64_t i = 0; i < count; ++i) {
		uint32_t dataSize = 0;
		in.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

		std::string data;
		data.resize(dataSize);
		if (dataSize > 0) {
			in.read(&data[0], static_cast<std::streamsize>(dataSize));
		}

		int64_t randIndex = -1;
		in.read(reinterpret_cast<char*>(&randIndex), sizeof(randIndex));

		if (!in) {
			for (ListNode* n : nodes) delete n;
			return nullptr;
		}

		ListNode* node = new ListNode();
		node->data = std::move(data);
		nodes.push_back(node);
		randIndexes.push_back(randIndex);
	}

	for (size_t i = 0; i < nodes.size(); ++i) {
		if (i > 0) {
			nodes[i]->prev = nodes[i - 1];
		}
		if (i + 1 < nodes.size()) {
			nodes[i]->next = nodes[i + 1];
		}

		int64_t r = randIndexes[i];
		if (r >= 0 && static_cast<uint64_t>(r) < count) {
			nodes[i]->rand = nodes[static_cast<size_t>(r)];
		}
	}

	return nodes.front();
}

static size_t listSize(ListNode* head) {
	size_t n = 0;
	for (ListNode* p = head; p != nullptr; p = p->next) ++n;
	return n;
}

static int64_t indexOfNode(ListNode* head, ListNode* target) {
	if (target == nullptr) return -1;
	int64_t i = 0;
	for (ListNode* p = head; p != nullptr; p = p->next, ++i) {
		if (p == target) return i;
	}
	return -1;
}

static void printList(ListNode* head, const std::string& title, size_t maxRows = 20) {
	size_t total = listSize(head);
	std::cout << title << " (" << total << " nodes)\n";
	if (total == 0) {
		std::cout << "  <empty>\n";
		return;
	}
	std::cout << "  " << std::left
		<< std::setw(6)  << "idx"
		<< std::setw(40) << "data"
		<< std::setw(8)  << "rand"
		<< "\n";
	std::cout << "  " << std::string(54, '-') << "\n";

	size_t i = 0;
	for (ListNode* p = head; p != nullptr; p = p->next, ++i) {
		if (i == maxRows && total > maxRows + 1) {
			std::cout << "  ... (" << (total - maxRows) << " more nodes) ...\n";
			break;
		}
		std::string data = p->data;
		if (data.size() > 36) data = data.substr(0, 33) + "...";
		std::cout << "  " << std::left
			<< std::setw(6)  << i
			<< std::setw(40) << ("\"" + data + "\"")
			<< std::setw(8)  << indexOfNode(head, p->rand)
			<< "\n";
	}
}

static bool listsEqual(ListNode* a, ListNode* b) {
	while (a != nullptr && b != nullptr) {
		if (a->data != b->data) return false;
		if (indexOfNode(a, a->rand) != indexOfNode(b, b->rand)) return false;
		if ((a->prev == nullptr) != (b->prev == nullptr)) return false;
		if ((a->next == nullptr) != (b->next == nullptr)) return false;
		a = a->next;
		b = b->next;
	}
	return a == nullptr && b == nullptr;
}

static void hexPreview(const std::string& path, size_t maxBytes = 96) {
	std::ifstream f(path, std::ios::binary | std::ios::ate);
	if (!f.is_open()) {
		std::cout << "  <cannot open " << path << ">\n";
		return;
	}
	std::streamsize total = f.tellg();
	f.seekg(0, std::ios::beg);
	size_t toRead = static_cast<size_t>(total);
	bool truncated = false;
	if (toRead > maxBytes) { toRead = maxBytes; truncated = true; }
	std::vector<unsigned char> buf(toRead);
	if (toRead > 0) f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(toRead));

	std::cout << "Hex preview of " << path
		<< " (" << total << " bytes" << (truncated ? ", first " + std::to_string(maxBytes) + " shown" : "") << ")\n";
	for (size_t i = 0; i < buf.size(); i += 16) {
		std::ostringstream hex, ascii;
		hex << "  " << std::setw(8) << std::setfill('0') << std::hex << i << "  ";
		for (size_t j = 0; j < 16; ++j) {
			if (i + j < buf.size()) {
				hex << std::setw(2) << std::setfill('0') << std::hex
					<< static_cast<int>(buf[i + j]) << ' ';
				unsigned char c = buf[i + j];
				ascii << (c >= 32 && c < 127 ? static_cast<char>(c) : '.');
			} else {
				hex << "   ";
			}
		}
		std::cout << hex.str() << " " << ascii.str() << "\n";
	}
	std::cout << std::dec << std::setfill(' ');
}

int main() {
	std::cout << "===== Linked List Serialization =====\n\n";

	std::ifstream input("inlet.in", std::ios::binary);
	if (!input.is_open()) {
		std::cerr << "[ERROR] Cannot open inlet.in (expected next to the .exe / in the working directory)\n";
		return 1;
	}

	std::cout << "[1/4] Reading inlet.in ...\n";
	ListNode* head = buildListFromText(input);
	input.close();

	printList(head, "      Parsed list");
	std::cout << "\n";

	std::cout << "[2/4] Serializing to outlet.out ...\n";
	{
		std::ofstream output("outlet.out", std::ios::binary);
		if (!output.is_open()) {
			std::cerr << "[ERROR] Cannot open outlet.out for writing\n";
			freeList(head);
			return 1;
		}
		serialize(head, output);
	}

	std::streamsize outSize = -1;
	{
		std::ifstream sizeProbe("outlet.out", std::ios::binary | std::ios::ate);
		if (sizeProbe.is_open()) {
			outSize = static_cast<std::streamsize>(sizeProbe.tellg());
		}
	}
	std::cout << "      Wrote outlet.out (" << outSize << " bytes)\n\n";

	hexPreview("outlet.out");
	std::cout << "\n";

	std::cout << "[3/4] Reading outlet.out back (deserialize) ...\n";
	std::ifstream verify("outlet.out", std::ios::binary);
	ListNode* head2 = deserialize(verify);
	verify.close();
	printList(head2, "      Restored list");
	std::cout << "\n";

	std::cout << "[4/4] Round-trip check ...\n";
	bool ok = listsEqual(head, head2);
	std::cout << "      " << (ok ? "OK  - restored list matches the original"
		: "FAIL - restored list differs from the original") << "\n\n";

	freeList(head);
	freeList(head2);

	std::cout << (ok ? "Serialization completed successfully.\n"
		: "Serialization finished, but round-trip FAILED.\n");
	return ok ? 0 : 2;
}
