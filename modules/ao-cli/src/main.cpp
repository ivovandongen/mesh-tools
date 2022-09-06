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
    uint8_t blurKernelSize;
    bool verbose;
};

Options parseOpts(int argc, char** argv) {
    cxxopts::Options options(argv[0], "Tiler");

    // clang-format off
    options.add_options()
            ("i,input", "Input model file", cxxopts::value<std::string>())
            ("o,output", "Output model file", cxxopts::value<std::string>())
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

    // Create UV Atlas
    logging::info("Create UV Atlas");
    // TODO: cmd line params for atlas create options
    uv::AtlasCreateOptions atlasOptions{
            .maxSize = 0,
            .padding = 4,
            .texelsPerUnit = 0,
            .resolution = 512,
            .blockAlign = true,
            .bruteForce = false,
    };
    auto atlasResult = uv::Atlas::Create(*modelLoadResult.value, atlasOptions);
    if (!atlasResult) {
        logging::error("Could create UV atlas for model {}: {}", options.input.c_str(), atlasResult.error.c_str());
        return EXIT_FAILURE;
    }

    // Apply atlas to model
    logging::info("Applying UV Atlas");
    atlasResult.value->apply(*modelLoadResult.value);

    // Bake AO
    logging::info("Baking AO");
    // TODO: Make sure we bake npot textures / pre-defined texture sizes
    auto bakeResult = ao::bake(*modelLoadResult.value, *atlasResult.value, {});
    if (!bakeResult) {
        logging::error("Could bake AO for model {}: {}", options.input.c_str(), bakeResult.error.c_str());
        return EXIT_FAILURE;
    }

    // Blur texture
    if (options.blurKernelSize > 0) {
        logging::info("Blur pass. Kernel size {}", options.blurKernelSize);
        blur(*bakeResult.value, options.blurKernelSize);
    }

    // Output
    if (!options.outputTexture.empty()) {
        logging::info("Writing texture to {}", options.outputTexture.c_str());
        writePng(*bakeResult.value, options.outputTexture);
    }
    logging::info("Writing result to {}", options.output.c_str());
    modelLoadResult.value->dump(*bakeResult.value, options.output);

    return EXIT_SUCCESS;
}