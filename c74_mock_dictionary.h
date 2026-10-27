/// @file
///    @ingroup     minapi
///    @copyright    Copyright 2018 The Min-API Authors. All rights reserved.
///    @license    Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74 {
namespace max {

    using t_dictionary = t_object;

    /// Dictionaries registered by name. Entries are not removed when a dictionary is freed.
    static std::unordered_map<t_symbol*, t_dictionary*> g_mock_registered_dictionaries;

    /// Number of dictobj_release() calls, so tests can tell it apart from object_free().
    static long g_mock_dictobj_release_count = 0;

    MOCK_EXPORT long mock_dictobj_release_count() {
        return g_mock_dictobj_release_count;
    }

    MOCK_EXPORT t_dictionary* dictobj_findregistered_retain(t_symbol* name) {
        auto registered = g_mock_registered_dictionaries.find(name);
        if (registered == g_mock_registered_dictionaries.end())
            return nullptr;
        auto refcount = g_mock_refcounts.find(registered->second);
        if (refcount == g_mock_refcounts.end())
            return nullptr;
        ++refcount->second;
        return registered->second;
    }

    /// Dictionaries are the only reference counted mock objects, so a counted object is a dictionary.
    MOCK_EXPORT long atomisdictionary(t_atom* a) {
        return a->a_type == A_OBJ && g_mock_refcounts.count(a->a_w.w_obj);
    }

    MOCK_EXPORT t_max_err dictobj_release(t_dictionary* d) {
        ++g_mock_dictobj_release_count;
        return object_free(d);
    }

    struct t_mock_dictionary_entry {
        t_symbol*           key;
        std::vector<t_atom> atoms;
        std::string         string; ///< set instead of atoms by dictionary_appendstring()
    };

    /// Entries of each dictionary in insertion order.
    static std::unordered_map<t_dictionary*, std::vector<t_mock_dictionary_entry>> g_mock_dictionary_entries;

    static t_mock_dictionary_entry* mock_dictionary_find(const t_dictionary* d, t_symbol* key) {
        auto entries = g_mock_dictionary_entries.find(const_cast<t_dictionary*>(d));
        if (entries == g_mock_dictionary_entries.end())
            return nullptr;
        for (auto& entry : entries->second) {
            if (entry.key == key)
                return &entry;
        }
        return nullptr;
    }

    /// Free the dictionaries an entry holds, as Max does when a value is replaced or deleted.
    static void mock_dictionary_free_value(t_mock_dictionary_entry& entry) {
        for (auto& a : entry.atoms) {
            if (a.a_type == A_OBJ && g_mock_refcounts.count(a.a_w.w_obj))
                object_free(a.a_w.w_obj);
        }
    }

    static t_max_err mock_dictionary_put(t_dictionary* d, t_symbol* key, std::vector<t_atom> atoms, std::string string = {}) {
        auto entry = mock_dictionary_find(d, key);
        if (entry)
            mock_dictionary_free_value(*entry);
        else
            entry = &g_mock_dictionary_entries[d].emplace_back(t_mock_dictionary_entry{ key });
        entry->atoms = std::move(atoms);
        entry->string = std::move(string);
        return 0;
    }

    MOCK_EXPORT t_dictionary* dictionary_new(void) {
        auto d = static_cast<t_dictionary*>(calloc(1, sizeof(t_dictionary)));
        g_mock_refcounts[d] = 1;
        g_mock_dictionary_entries.erase(d); // left over from a freed dictionary at the same address
        return d;
    }

    MOCK_EXPORT t_max_err dictionary_getatoms(t_dictionary* d, t_symbol* key, long* argc, t_atom** argv) {
        auto entry = mock_dictionary_find(d, key);
        if (!entry)
            return -1;
        *argc = static_cast<long>(entry->atoms.size());
        *argv = entry->atoms.data();
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_appendatoms(t_dictionary* d, t_symbol* key, long argc, t_atom* argv) {
        return mock_dictionary_put(d, key, { argv, argv + argc });
    }

    MOCK_EXPORT t_max_err dictionary_appendatom(t_dictionary* d, t_symbol* key, t_atom* value) {
        return mock_dictionary_put(d, key, { *value });
    }

    MOCK_EXPORT t_max_err dictionary_appendatomarray(t_dictionary* d, t_symbol* key, t_object* value) {
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_appenddictionary(t_dictionary* d, t_symbol* key, t_object* value) {
        t_atom a;
        atom_setobj(&a, value);
        return mock_dictionary_put(d, key, { a });
    }

    MOCK_EXPORT t_max_err dictionary_appendlong(t_dictionary* d, t_symbol* key, t_atom_long value) {
        t_atom a;
        atom_setlong(&a, value);
        return mock_dictionary_put(d, key, { a });
    }

    MOCK_EXPORT t_max_err dictionary_appendfloat(t_dictionary* d, t_symbol* key, double value) {
        t_atom a;
        atom_setfloat(&a, value);
        return mock_dictionary_put(d, key, { a });
    }

    MOCK_EXPORT t_max_err dictionary_appendsym(t_dictionary* d, t_symbol* key, t_symbol* value) {
        t_atom a;
        atom_setsym(&a, value);
        return mock_dictionary_put(d, key, { a });
    }

    MOCK_EXPORT t_max_err dictionary_appendstring(t_dictionary* d, t_symbol* key, const char* value) {
        return mock_dictionary_put(d, key, {}, value);
    }

    MOCK_EXPORT long dictionary_hasentry(t_dictionary* d, t_symbol* key) {
        return mock_dictionary_find(d, key) != nullptr;
    }

    MOCK_EXPORT t_atom_long dictionary_getentrycount(t_dictionary* d) {
        auto entries = g_mock_dictionary_entries.find(d);
        return entries == g_mock_dictionary_entries.end() ? 0 : static_cast<t_atom_long>(entries->second.size());
    }

    MOCK_EXPORT t_max_err dictionary_getkeys_ordered(t_dictionary* d, long* numkeys, t_symbol*** keys) {
        *numkeys = static_cast<long>(dictionary_getentrycount(d));
        *keys = nullptr;
        if (*numkeys) {
            *keys = static_cast<t_symbol**>(malloc(*numkeys * sizeof(t_symbol*)));
            for (long i = 0; i < *numkeys; ++i)
                (*keys)[i] = g_mock_dictionary_entries[d][i].key;
        }
        return 0;
    }

    MOCK_EXPORT void dictionary_freekeys(t_dictionary* d, long numkeys, t_symbol** keys) {
        free(keys);
    }

    MOCK_EXPORT t_max_err dictionary_chuckentry(t_dictionary* d, t_symbol* key) {
        auto entries = g_mock_dictionary_entries.find(d);
        if (entries == g_mock_dictionary_entries.end())
            return -1;
        auto& v = entries->second;
        auto entry = std::find_if(v.begin(), v.end(), [key](const t_mock_dictionary_entry& e) { return e.key == key; });
        if (entry == v.end())
            return -1;
        v.erase(entry);
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_deleteentry(t_dictionary* d, t_symbol* key) {
        auto entry = mock_dictionary_find(d, key);
        if (!entry)
            return -1;
        mock_dictionary_free_value(*entry);
        return dictionary_chuckentry(d, key);
    }

    /// Reads the first atom of the entry, so a stored list yields its first item.
    static const t_atom* mock_dictionary_first_atom(const t_dictionary* d, t_symbol* key) {
        auto entry = mock_dictionary_find(d, key);
        return entry && !entry->atoms.empty() ? entry->atoms.data() : nullptr;
    }

    MOCK_EXPORT t_max_err dictionary_getdeflong(const t_dictionary* d, t_symbol* key, t_atom_long* value, t_atom_long def) {
        auto a = mock_dictionary_first_atom(d, key);
        *value = a ? atom_getlong(a) : def;
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_getdeffloat(const t_dictionary* d, t_symbol* key, double* value, double def) {
        auto a = mock_dictionary_first_atom(d, key);
        *value = a ? atom_getfloat(a) : def;
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_getdefsym(const t_dictionary* d, t_symbol* key, t_symbol** value, t_symbol* def) {
        auto a = mock_dictionary_first_atom(d, key);
        *value = a ? atom_getsym(a) : def;
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_getdefstring(const t_dictionary* d, t_symbol* key, const char** value, char* def) {
        auto entry = mock_dictionary_find(d, key);
        *value = entry ? entry->string.c_str() : def;
        return 0;
    }

    MOCK_EXPORT t_dictionary* dictobj_register(t_dictionary* d, t_symbol** name) {
        if (*name)
            g_mock_registered_dictionaries[*name] = d;
        return d;
    }


    MOCK_EXPORT t_max_err dictionary_clone_to_existing(const t_dictionary* d, t_dictionary* dc) {
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_copyunique(t_dictionary* d, t_dictionary* copyfrom) {
        return 0;
    }


    MOCK_EXPORT t_symbol* dictobj_namefromptr(t_dictionary* d) {
        return nullptr;
    }

    MOCK_EXPORT t_max_err dictobj_dictionaryfromatoms(t_dictionary** d, const long argc, const t_atom* argv) {
        *d = dictionary_new();
        return 0;
    }

    MOCK_EXPORT t_max_err object_notify(void* x, const t_symbol* s, void* data) {
        return 0;
    }



}} // namespace c74::max
