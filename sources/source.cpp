// Copyright 2018 Your Name <your_email>
#include <header.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

int main(int argc, char* argv[]){
    boost::shared_ptr< logging::core > core = logging::core::get();
    // File sink
    boost::shared_ptr< sinks::text_file_backend > fileBackend =
            boost::make_shared< sinks::text_file_backend >(
                    keywords::file_name = "/home/rooted/sample_%N.log",
                    keywords::rotation_size = 10 * 1024 * 1024,
                    keywords::time_based_rotation =
                            sinks::file::rotation_at_time_point(0, 0, 0),
                    keywords::format = "[%TimeStamp%]: %Message%"
    );
    typedef sinks::synchronous_sink< sinks::text_file_backend > fileSink_t;
    boost::shared_ptr< fileSink_t > fileSink(new fileSink_t(fileBackend));
    core->add_sink(fileSink);
    // Console sink
    boost::shared_ptr< sinks::text_ostream_backend> backend =
            boost::make_shared< sinks::text_ostream_backend >();
    backend->add_stream(
            boost::shared_ptr< std::ostream >(&std::cout));
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t(backend));
    core->add_sink(sink);
    logging::add_common_attributes();
    using namespace logging::trivial;
    src::severity_logger<severity_level> lg;

    std::srand(static_cast<unsigned int>(std::time(nullptr))); // NOLINT(cert-msc32-c,cert-msc51-cpp)

    unsigned int threadCount;

    if (argc > 1) {
        threadCount = static_cast<unsigned int>(std::stoi(argv[1]));
    }
    else{
        threadCount = std::thread::hardware_concurrency();
    }

    for (unsigned int j = 0; j < threadCount; ++j) {

        auto thread = std::thread([&lg](){

            while(true) {

                std::vector<char> data(100);
                std::generate(data.begin(), data.end(), []() {
                    auto x = 31;
                    while (x < 32 || x > 125) {

                        x = 32 + std::rand() / ((RAND_MAX + 1u) / 94); //NOLINT(cert-msc30-c, cert-msc51-cpp)

                    }
                    return x;
                });

                auto sha = picosha2::hash256_hex_string(data);

                BOOST_LOG_SEV(lg, trace) << "Thread with id: " << std::this_thread::get_id() << " hashing data: "
                                         << std::accumulate(data.begin(), data.end(), std::string()) << " found: "
                                         << sha;

                if (sha.substr(sha.length() - 5) == "0000") {
                    BOOST_LOG_SEV(lg, info) << "Hash found, data is: "
                                            << std::accumulate(data.begin(), data.end(), std::string());
                }

            }

        });

        thread.detach();

    }

    using namespace std::chrono_literals;

    while (true){

        std::this_thread::sleep_for(1s);

    }

}