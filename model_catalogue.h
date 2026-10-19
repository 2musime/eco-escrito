#pragma once

// The Whisper models the interface offers, and the directories a model is
// looked for in. The interface and the worker both read this, so a model the
// interface offers is always one the worker will find.

#include <filesystem>
#include <string>
#include <vector>
#include "file_storage.h"

struct CatalogueModel {
    const char* name;         // what the install scripts call it
    const char* file;         // what they save it as
    const char* label;        // what the interface calls it
    const char* description;  // one line on when to choose it
};

// Fastest first. The release packages install both (ECO_ESCRITO_BUNDLE_MODEL
// in CMakeLists.txt), and each name is one the install scripts accept, for a
// build made without them.
inline const std::vector<CatalogueModel>& catalogue_models() {
    static const std::vector<CatalogueModel> models{
        {"base.en", "ggml-base.en.bin", "Faster",
         "Suits most people."},
        {"small.en", "ggml-small.en.bin", "More accurate",
         "About 3.5 times slower than Faster."},
    };
    return models;
}

inline const char* default_catalogue_model() { return "ggml-base.en.bin"; }

// Where a model may live, in search order. The command line can approve more
// with --model-dir; these are the ones that need no approval.
inline std::vector<fs::path> standard_model_roots(const fs::path& data_directory) {
    std::vector<fs::path> roots{
        fs::current_path() / "models",
        data_directory / "models"
    };
    // A model shipped beside the application. This is how a package that
    // includes one supplies it, and it is the only root that does not depend on
    // the working directory -- which nothing sets when a program is started
    // from a menu entry or a desktop shortcut.
    const fs::path beside_executable = executable_directory();
    if (!beside_executable.empty()) {
        roots.push_back(beside_executable / "models");
    }
#if !defined(_WIN32)
    // Where install-model.sh --system puts a model. The prefix comes from the
    // executable, so an installation under /usr/local finds its own models;
    // /usr/share is what the script writes to whatever the prefix.
    if (!beside_executable.empty()) {
        roots.push_back(beside_executable.parent_path() / "share" / "eco-escrito" / "models");
    }
    roots.emplace_back("/usr/share/eco-escrito/models");
#endif
    return roots;
}

// The first root holding a regular file by this name, or an empty path.
inline fs::path find_catalogue_model(const std::string& file, const std::vector<fs::path>& roots) {
    for (const fs::path& root : roots) {
        std::error_code error;
        const fs::path candidate = root / file;
        if (fs::is_regular_file(candidate, error) && !error) {
            return candidate;
        }
    }
    return {};
}
