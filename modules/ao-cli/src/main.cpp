#include <meshtools/ao/ao.hpp>
#include <meshtools/image.hpp>
#include <meshtools/logging.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/uv/atlas.hpp>

#include <cxxopts.hpp>

#include <filesystem>

using namespace meshtools;

struct Options {
    std::filesystem::path input;
    std::filesystem::path output;
    std::filesystem::path outputTexture;
    uint32_t resolution;
    uint8_t blurKernelSize;
    bool verbose;
};

Options parseOpts(int argc, char** argv) {
    cxxopts::Options options(argv[0], "Tiler");

    // clang-format off
    options.add_options()
            ("i,input", "Input model file", cxxopts::value<std::string>())
            ("o,output", "Output model file", cxxopts::value<std::string>())
            ("r,resolution", "Output texture resolution", cxxopts::value<uint32_t>()->default_value("0"))
            ("output-texture", "Output texture file separately", cxxopts::value<std::string>()->default_value(""))
            ("b,blur", "Blur kernel size", cxxopts::value<uint8_t>()->default_value("5"))
            ("v,verbose", "Speak up!", cxxopts::value<bool>()->default_value("false"))
            ("h,help","Print usage");
    // clang-format on

    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            logging::info(options.help().c_str());
            exit(0);
        }

        Options opts{
                result["input"].as<std::string>(),
                result["output"].as<std::string>(),
                result["output-texture"].as<std::string>(),
                result["resolution"].as<uint32_t>(),
                result["blur"].as<uint8_t>(),
                result["verbose"].as<bool>(),
        };

        return opts;
    } catch (const cxxopts::exceptions::exception& e) {
        logging::error("Invalid options: {}", e.what());
        logging::info(options.help().c_str());
        exit(1);
    } catch (...) {
        logging::info(options.help().c_str());
        exit(1);
    }
}

int main(int argc, char** argv) {
    auto options = parseOpts(argc, argv);
    if (options.verbose) {
        logging::setLevel(logging::Level::DEBUG);
    }

    // Load input model
    logging::info("Loading {}", options.input.c_str());
    auto modelLoadResult = models::Model::Load(options.input);
    if (!modelLoadResult) {
        logging::error("Could not load model {}: {}", options.input.c_str(), modelLoadResult.error.c_str());
        return EXIT_FAILURE;
    }

    auto resolution = options.resolution > 0 ? Size<uint32_t>{options.resolution, options.resolution} : Size<uint32_t>{};
    {
        // Create UV Atlas
        auto atlasRes = resolution.width > 0 ? resolution.width : 512;
        logging::info("Create UV Atlas. Resolution {}x{}", atlasRes, atlasRes);
        // TODO: cmd line params for atlas create options
        uv::AtlasCreateOptions atlasOptions{
                .maxSize = 0,
                .padding = 4,
                .texelsPerUnit = 0,
                .resolution = atlasRes,
                .blockAlign = true,
                .bruteForce = false,
        };
        auto atlasResult = uv::Atlas::Create(*modelLoadResult.value, atlasOptions);
        if (!atlasResult) {
            logging::error("Could create UV atlas for model {}: {}", options.input.c_str(), atlasResult.error.c_str());
            return EXIT_FAILURE;
        }

        logging::info("Created UV Atlas. Resolution {}x{}", atlasResult.value->width(), atlasResult.value->height());
        if (resolution.width == 0) {
            // If resolution was not explicitly set, use the atlas's native resolution
            resolution = {atlasResult.value->width(), atlasResult.value->height()};
        }

        // Apply atlas to model
        logging::info("Applying UV Atlas");
        atlasResult.value->apply(*modelLoadResult.value);
    }

    // Bake AO
    logging::info("Baking AO. Resolution {}x{}", resolution.width, resolution.height);
    auto bakeResult = ao::bake(*modelLoadResult.value, resolution, {});
    if (!bakeResult) {
        logging::error("Could bake AO for model {}: {}", options.input.c_str(), bakeResult.error.c_str());
        return EXIT_FAILURE;
    }

    // Blur texture
    if (options.blurKernelSize > 0) {
        logging::info("Blur pass. Kernel size {}", options.blurKernelSize);
        bakeResult.value->blur(options.blurKernelSize);
    }

    // Output
    if (!options.outputTexture.empty()) {
        logging::info("Writing texture to {}", options.outputTexture.c_str());
        bakeResult.value->png(options.outputTexture);
    }

    logging::info("Writing result to {}", options.output.c_str());
    modelLoadResult.value->dump(*bakeResult.value, options.output);

    return EXIT_SUCCESS;
}