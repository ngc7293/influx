#include <thread>

#include <gtest/gtest.h>

#include <influx/influx.hh>

#include "docker.hh"

using namespace std::literals::chrono_literals;

namespace influx::test {
    nlohmann::json docker_config;
}

int main(int argc, const char* argv[])
{
    ::testing::InitGoogleTest(&argc, (char**) argv);

    bool use_container = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use-container") == 0) {
            use_container = true;
        }
    }

    if (use_container) {
        docker::Docker dock;
        auto id = dock.Launch(
            "influxdb:latest", 
            {{docker::Protocol::TCP, 8086}},
            {
                {"DOCKER_INFLUXDB_INIT_MODE", "setup"},
                {"DOCKER_INFLUXDB_INIT_USERNAME", "admin"},
                {"DOCKER_INFLUXDB_INIT_PASSWORD", "influx-container-password"},
                {"DOCKER_INFLUXDB_INIT_ORG", "test-org"},
                {"DOCKER_INFLUXDB_INIT_BUCKET", "test-bucket"},
                {"DOCKER_INFLUXDB_INIT_ADMIN_TOKEN", "influx-container-token"}
            }
        );


        int rc = -1;
        try {
            if (id) {
                auto ip = dock.Get("/containers/" + id.value() + "/json").content["NetworkSettings"]["IPAddress"].get<std::string>();
                
                auto db = influx::Influx("http://" + ip + ":8086", "", "influx-container-token");
                while(!db.Ready()) {
                    std::this_thread::sleep_for(500ms);
                }

                influx::test::docker_config["host"] = "http://" + ip + ":8086";
                influx::test::docker_config["org"] = db.OrgIdFromName("test-org");
                influx::test::docker_config["token"] = "influx-container-token";
            } else {
                std::cerr << "Failed to launch Influx Container";
                return EXIT_FAILURE;
            }

            rc = RUN_ALL_TESTS();
        } catch (const std::exception& e) {
            std::cerr << "FAILED: " << e.what() << std::endl;
        }

        dock.Stop(id.value());
        dock.Remove(id.value());

        return rc;
    } else {
        return RUN_ALL_TESTS();
    } 
}