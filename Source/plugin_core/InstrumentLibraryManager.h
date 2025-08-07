// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <cctype>

namespace spawnclone::core {

// Minimal, dependency-light instrument registry for Sprint 1.
// Provides registration, lookup by id, tag queries, and listing.
class InstrumentLibraryManager {
public:
    struct Instrument {
        std::string id;                 // unique key
        std::string name;               // human readable
        std::vector<std::string> tags;  // arbitrary metadata tags

        bool operator==(const Instrument& other) const noexcept {
            return id == other.id && name == other.name && tags == other.tags;
        }
    };

    // Register or update an instrument by id. Returns true if newly inserted, false if updated.
    bool registerInstrument(Instrument inst) {
        normalize(inst);
        const bool inserted = (registry.find(inst.id) == registry.end());
        registry[inst.id] = inst;
        // update tag index
        indexTags(inst);
        return inserted;
    }

    // Remove by id. Returns true if removed.
    bool removeInstrument(const std::string& id) {
        auto it = registry.find(id);
        if (it == registry.end()) return false;
        // remove from tag index
        unindexTags(it->second);
        registry.erase(it);
        return true;
    }

    bool hasInstrument(const std::string& id) const {
        return registry.find(id) != registry.end();
    }

    std::optional<Instrument> getInstrument(const std::string& id) const {
        auto it = registry.find(id);
        if (it == registry.end()) return std::nullopt;
        return it->second;
    }

    std::vector<Instrument> listInstruments() const {
        std::vector<Instrument> out;
        out.reserve(registry.size());
        for (const auto& kv : registry) out.push_back(kv.second);
        // stable sort by name for deterministic tests
        std::stable_sort(out.begin(), out.end(), [](const Instrument& a, const Instrument& b){
            return a.name < b.name;
        });
        return out;
    }

    // Returns instruments that contain the given tag (case-insensitive match).
    std::vector<Instrument> findByTag(const std::string& tag) const {
        const std::string norm = toLower(tag);
        std::vector<Instrument> out;
        auto it = tagIndex.find(norm);
        if (it == tagIndex.end()) return out;
        out.reserve(it->second.size());
        for (const auto& id : it->second) {
            auto rit = registry.find(id);
            if (rit != registry.end()) out.push_back(rit->second);
        }
        std::stable_sort(out.begin(), out.end(), [](const Instrument& a, const Instrument& b){
            return a.name < b.name;
        });
        return out;
    }

    void clear() {
        registry.clear();
        tagIndex.clear();
    }

private:
    // Helpers implemented inline to avoid out-of-class qualified definitions.
    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static std::string trim(const std::string& s) {
        auto isspacep = [](unsigned char c){ return std::isspace(c) != 0; };
        size_t start = 0;
        while (start < s.size() && isspacep(static_cast<unsigned char>(s[start]))) ++start;
        if (start == s.size()) return std::string();
        size_t end = s.size() - 1;
        while (end > start && isspacep(static_cast<unsigned char>(s[end]))) --end;
        return s.substr(start, end - start + 1);
    }

    static void normalize(Instrument& inst) {
        // Trim/normalize id and tags lightly
        inst.id = trim(inst.id);
        inst.name = trim(inst.name);
        for (auto& t : inst.tags) {
            t = trim(toLower(t));
        }
        // Deduplicate tags
        std::sort(inst.tags.begin(), inst.tags.end());
        inst.tags.erase(std::unique(inst.tags.begin(), inst.tags.end()), inst.tags.end());
    }

    void indexTags(const Instrument& inst) {
        // rebuild indices for this instrument: remove first then add
        unindexTags(inst);
        for (const auto& t : inst.tags) {
            tagIndex[t].insert(inst.id);
        }
    }

    void unindexTags(const Instrument& inst) {
        for (const auto& t : inst.tags) {
            auto it = tagIndex.find(t);
            if (it != tagIndex.end()) {
                it->second.erase(inst.id);
                if (it->second.empty()) tagIndex.erase(it);
            }
        }
    }

    std::unordered_map<std::string, Instrument> registry;
    std::unordered_map<std::string, std::unordered_set<std::string>> tagIndex;
};

} // namespace spawnclone::core
