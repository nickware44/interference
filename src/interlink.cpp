/////////////////////////////////////////////////////////////////////////////
// Name:        interlink.cpp
// Purpose:     Interlink service class
// Author:      Nickolay Babbysh
// Created:     19.07.2023
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <inn/interlink.h>
#include <inn/system.h>
#include <json.hpp>
#include <httplib.h>
#include <unistd.h>

typedef nlohmann::json json;

inn::Interlink::Interlink() {
    Input = nullptr;
    Output = nullptr;
    Interlinked.store(false);
    doInitInput(4408);
}

inn::Interlink::Interlink(int port) {
    Input = nullptr;
    Output = nullptr;
    Interlinked.store(false);
    doInitInput(port);
}

void inn::Interlink::doInitInput(int port) {
    Input = new httplib::Server();
    auto input = (httplib::Server*)Input;

    input-> set_idle_interval(60, 0);
    input -> set_read_timeout(60, 0);
    input -> set_write_timeout(60, 0);
    input-> set_keep_alive_timeout(60);

    input -> Post("/io_init", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
        Host = req.remote_addr;
        std::string body;

        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });

        auto j = json::parse(body);
        OutputPort = j["port"].get<std::string>();
        Structure = j["structure"].get<std::string>();

        doInitOutput();
    });

    input -> Post("/io_ping", [&](const httplib::Request &req, httplib::Response &res) {
    });

    input -> Post("/io_model_write_structure", [&](const httplib::Request &req, httplib::Response &res) {
        Structure = "";
        res.set_content_provider(
                "text/plain",
                [&](size_t offset, httplib::DataSink &sink) {
                    sink.write(Structure.c_str(), Structure.size());
                    sink.done();
                    return true;
                });
    });

    Thread = std::thread([this, port, input]() {
        input -> listen("0.0.0.0", port);
    });

    int timeout = 0;
    while (!Interlinked.load() && timeout < 5) {
        sleep(1);
        timeout++;
        if (inn::System::getVerbosityLevel() > 1)
            std::cout << "Interlink connection timeout " << timeout << std::endl;
    }
}

void inn::Interlink::doInitOutput() {
    if (inn::System::getVerbosityLevel() > 0)
        std::cout << "Incoming Interlink connection from " << Host+":"+OutputPort << std::endl;
    Output = new httplib::Client(Host+":"+OutputPort);
    auto output = (httplib::Client*)Output;
    output -> set_read_timeout(5, 0);
    output -> set_write_timeout(5, 0);
    output -> set_connection_timeout(0, 500000);
    Interlinked.store(true);
}

void inn::Interlink::doSend(const std::string& command, const std::string& data) {
    if (!Interlinked.load() || !Output) return;
    auto output = (httplib::Client*)Output;
    auto res = output -> Post("/"+command, data.size(),
                   [data](size_t offset, size_t length, httplib::DataSink &sink) {
                        sink.write(data.c_str()+offset, length);
                        return true;
                   },
                   "text/plain");

//    std::cout << "Sent command " << command << std::endl;
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (!res || res && res->status != 200) {
        std::cout << "Send failed " << command << std::endl;
        Interlinked.store(false);
    }
}

void inn::Interlink::doUpdateStructure(const std::string &data) {
    doSend("io_app_update_structure", data);
}

void inn::Interlink::doUpdateData(const std::string &data) {
    doSend("io_app_update_data", data);
}

void inn::Interlink::setStructure(const std::string &data) {
    doSend("io_app_write_structure", data);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

std::string inn::Interlink::getStructure() {
    return Structure;
}

bool inn::Interlink::isInterlinked() {
    return Interlinked.load();
}

inn::Interlink::~Interlink() {
    Thread.detach();
    delete (httplib::Server*)Input;
    delete (httplib::Client*)Output;
}
