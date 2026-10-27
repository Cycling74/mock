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

    MOCK_EXPORT t_dictionary* dictionary_new(void) {
        auto d = static_cast<t_dictionary*>(calloc(1, sizeof(t_dictionary)));
        g_mock_refcounts[d] = 1;
        return d;
    }

    MOCK_EXPORT t_max_err dictionary_getatoms(t_dictionary* d, t_symbol* key, long* argc, t_atom** argv) {
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_appendatoms(t_dictionary* d, t_symbol* key, long argc, t_atom* argv) {
        return 0;
    } 

    MOCK_EXPORT t_max_err dictionary_appendatomarray(t_dictionary* d, t_symbol* key, t_object* value) {
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_appenddictionary(t_dictionary* d, t_symbol* key, t_object* value) {
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_appendlong(t_dictionary* d, t_symbol* key, t_atom_long value) {
        return 0;
    }

    MOCK_EXPORT long dictionary_hasentry(t_dictionary* d, t_symbol* key) {
        return 0;
    }

    MOCK_EXPORT t_max_err dictionary_chuckentry(t_dictionary* d, t_symbol* key) {
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
