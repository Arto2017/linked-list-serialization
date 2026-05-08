# Linked List Serialization

C++ implementation of serialization and deserialization for a doubly-linked list with arbitrary `rand` pointers (`prev` / `next` / `rand` + `data` per node).

## Task

Each `ListNode` has:

```cpp
struct ListNode {
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr; // points to any node in the list, or nullptr
    std::string data;
};
```

The program:

1. Reads `inlet.in` (text).
2. Builds a doubly-linked list in memory.
3. Serializes it to `outlet.out` (binary).
4. Reads the binary back (deserialize) and verifies a full round-trip on every run.

## Input format (`inlet.in`)

One node per line:

```
<data>;<rand_index>
```

- `<data>` — UTF-8 string. May contain spaces, special characters, and even `;` (the **last** `;` on the line is treated as the separator).
- `<rand_index>` — zero-based index of the node `rand` points to, or `-1` if `rand == nullptr`.

Example:

```
apple;2
banana;-1
carrot;1
```

Empty / malformed lines are silently skipped. CRLF line endings are handled.

## Binary format (`outlet.out`)

All integers are little-endian.

```
[ uint64 count ]
repeat count times:
    [ uint32 dataSize ]
    [ dataSize bytes  data (UTF-8) ]
    [ int64  randIndex ]   // -1 if rand == nullptr
```

File size: `8 + count * 12 + total_data_bytes`.

## Constraints (per task)

- Up to **10^6 nodes**
- `data` up to **1000 characters**
- `ListNode` struct must not be modified

## Complexity

- Parsing: O(n)
- Serialization: O(n) using `unordered_map<ListNode*, size_t>` for `rand -> index` lookup
- Deserialization: O(n)

A 1,000,000-node input file (~19 MB) is processed end-to-end in ~1.5 seconds.

## Build & run

### Visual Studio

Open `LinkedListSerialization.sln`, build (Ctrl+Shift+B), run (Ctrl+F5).

The working directory should be `$(ProjectDir)` so that `inlet.in` is found and `outlet.out` is written next to it (default for Visual Studio).

### Command line (MSVC)

```bat
cl /nologo /EHsc /std:c++17 /O2 LinkedListSerialization\main.cpp
main.exe
```

(Run from a "x64 Native Tools Command Prompt for VS 2022".)

## What you see when it runs

The program prints a 4-step report:

1. Parsed list (table of index / data / rand)
2. Bytes written to `outlet.out` + a hex preview
3. List restored from `outlet.out` (deserialization)
4. Round-trip check — must say `OK`

Exit code: `0` on success, `1` on I/O error, `2` if the round-trip check fails.

## Files

- `LinkedListSerialization/main.cpp` — full implementation (parser, serializer, deserializer, demo `main`).
- `LinkedListSerialization/inlet.in` — sample input.
- `LinkedListSerialization.sln` / `.vcxproj` — Visual Studio 2022 solution.
- `outlet.out` — generated on every run (gitignored).
