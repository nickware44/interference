/////////////////////////////////////////////////////////////////////////////
// Name:        neuralnet/neuralnet.cpp
// Purpose:     Neural net main class
// Author:      Nickolay Babbysh
// Created:     12.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <queue>
#include <thread>
#include <json.hpp>
#include <indk/neuralnet.h>

typedef nlohmann::json json;

indk::NeuralNet::NeuralNet() {
    t = 0;
    StateSyncEnabled = false;
    LastUsedComputeBackend = -1;
    InterlinkService = nullptr;
}

indk::NeuralNet::NeuralNet(const std::string &path) {
    t = 0;
    StateSyncEnabled = false;
    LastUsedComputeBackend = -1;
    InterlinkService = nullptr;
    std::ifstream filestream(path);
    setStructure(filestream);
}

void indk::NeuralNet::doInterlinkInit(int port) {
    InterlinkService = new indk::Interlink(port);
}

void indk::NeuralNet::doInterlinkAppUpdateData() {
    if (!InterlinkService || InterlinkService && !InterlinkService->isInterlinked()) return;
    json j;

    for (const auto &n: Neurons) {
        json jn;
        jn["name"] = n.second->getName();
        jn["time"] = n.second->getTime();

        for (int i = 0; i < n.second->getReceptorsCount(); i++) {
            json jr;
            auto r = n.second -> getReceptor(i);
            jr["sensitivity"] = r -> getSensitivityValue();

            auto scopes = r -> getReferencePosScopes();
            for (const auto &s: scopes) {
                json js;
                for (int p = 0; p < n.second->getDimensionsCount(); p++) {
                    js.push_back(s->getPositionValue(p));
                }
                jr["scopes"].push_back(js);
            }

            for (int p = 0; p < n.second->getDimensionsCount(); p++) {
                jr["phantom"].push_back(r->getPosf()->getPositionValue(p));
            }

            jn["receptors"].push_back(jr);
        }

        j["neurons"].push_back(jn);
    }

    InterlinkService -> doUpdateData(j.dump());
}

int64_t indk::NeuralNet::doFindEntry(const std::string& ename) {
    auto ne = std::find_if(Entries.begin(), Entries.end(), [ename](const std::pair<std::string, std::vector<std::string>>& e){
        return e.first == ename;
    });
    if (ne == Entries.end()) return -1;
    return std::distance(Entries.begin(), ne);
}

/**
 * Compare neuron patterns (learning and recognition patterns) for all output neurons.
 * @return Vector of pattern difference values for each output neuron.
 */
std::vector<float> indk::NeuralNet::doComparePatterns(int CompareFlag, int ProcessingMethod) {
    std::vector<float> PDiffR, PDiff;
    for (const auto& O: Outputs) {
        auto n = Neurons.find(O);
        if (n == Neurons.end()) break;
        auto P = n -> second -> doComparePattern(ProcessingMethod);
        PDiffR.push_back(std::get<0>(P));
    }

    switch (CompareFlag) {
        default:
        case indk::PatternCompareFlags::CompareDefault:
            return PDiffR;

        case indk::PatternCompareFlags::CompareNormalized:
            float PDRMin = PDiffR[std::distance(PDiffR.begin(), std::min_element(PDiffR.begin(), PDiffR.end()))];
            float PDRMax = PDiffR[std::distance(PDiffR.begin(), std::max_element(PDiffR.begin(), PDiffR.end()))] - PDRMin;
            for (auto &PDR: PDiffR) {
                if (PDRMax != 0) PDiff.push_back(1 - (PDR-PDRMin) / PDRMax);
                else PDiff.push_back(1);
            }
            return PDiff;
    }
}

void indk::NeuralNet::doCreateNewScope() {
    for (const auto& N: Neurons) N.second -> doCreateNewScope();
}

void indk::NeuralNet::doChangeScope(uint64_t scope) {
    for (const auto& N: Neurons) N.second -> doChangeScope(scope);
}

/**
 * Resets all neurons in the network.
 * See indk::Neuron::doReset() method for details.
 */
void indk::NeuralNet::doReset() {
    t = 0;
    for (const auto& N: Neurons) N.second -> doReset();
}

void indk::NeuralNet::doPrepare() {
    t = 0;
    for (const auto& N: Neurons) N.second -> doPrepare();
}

void indk::NeuralNet::doSignalProcessStart(const std::vector<std::vector<float>>& Xx, const EntryList& entries) {
    float value;
    int d = 0;

    int64_t dt = t;

    for (auto X: Xx) {
        int xi = 0;
        for (auto &e: entries) {
            for (auto &en: e.second) {
                auto n = Neurons.find(en);

                auto lto = Latencies.find(en);
                auto latencyto = lto != Latencies.end() ? lto->second : 0;
                auto waiting = n -> second -> getWaitingEntries();
                for (auto &we: waiting) {
                    auto nprev = Latencies.find(we);
                    if (nprev != Latencies.end() && nprev->second > latencyto) {
                        n -> second -> doSignalSendEntry(we, 0, 0);
                    }
                }

                if (n != Neurons.end())
                    n -> second -> doSignalSendEntry(e.first, X[xi], t);
            }
            xi++;
        }
        t++;
    }

    if (indk::System::getComputeBackendKind() == indk::System::ComputeBackends::OpenCL)
        indk::System::getComputeBackend() -> doWaitTarget();

    dt = t - dt;
    if (Xx.empty()) dt = 1;
    int64_t lt = 0;

    while (lt != dt) {
        d = 0;

        for (auto l: Links) {
            auto from = std::get<0>(l);
            auto to = std::get<1>(l);

            auto nfrom = (indk::Neuron*)std::get<2>(l);
            auto nto = (indk::Neuron*)std::get<3>(l);
            auto latency = std::get<4>(l);
            auto time = nto -> getTime();

            if (nto->getTime() == t) {
                d++;
                continue;
            }

            value = 0;
            if (!time && latency >= 0 || time) {
                auto shift = (bool)latency;
                if (latency > 0) shift = false;
                if (nfrom->getState(time-shift) != indk::Neuron::States::Computed) {
                    if (indk::System::getComputeBackendKind() == indk::System::ComputeBackends::OpenCL)
                        d++;
                    continue;
                }
                value = nfrom -> doSignalReceive(time-shift).second;
            }


            nto -> doSignalSendEntry(from, value, time);

            if (indk::System::getComputeBackendKind() == indk::System::ComputeBackends::OpenCL) {
                if (!Xx.empty() || Xx.empty() && time == t) {
                    d++;
                }
            } else if (time == t) {
                d++;
            }
        }

        if (d == Links.size()) {
            lt++;
        } else if (Xx.empty()) indk::System::getComputeBackend() -> doWaitTarget();
    }
}

void indk::NeuralNet::doParseLinks(const EntryList& entries, const std::string& id) {
    if (id == PrepareID) return;

    Links.clear();

    NQueue nqueue;

    for (auto &e: entries) {
        for (auto &en: e.second) {
            nqueue.emplace(e.first, en, nullptr, 0);
        }
    }

    while (!nqueue.empty()) {
        auto i = nqueue.front();
        nqueue.pop();

        auto from = std::get<0>(i);
        auto to = std::get<1>(i);
        auto latency = std::get<3>(i);

        auto n = Neurons.find(to);

        if (n != Neurons.end()) {
            bool skip = false;
            for (auto l: Links) {
                if (from == std::get<0>(l) && to == std::get<1>(l)) {
                    skip = true;
                    break;
                }
            }
            if (skip) continue;
            auto nprev = Neurons.find(from);
            auto type = 0;
            if (nprev != Neurons.end()) Links.emplace_back(from, to, nprev->second, n->second, latency);

            auto nlinks = n -> second -> getLinkOutput();
            for (auto &nl: nlinks) {
                auto shift = 0;
                auto nlatency = 0;
                auto lnext = Latencies.find(nl);
                if (lnext != Latencies.end()) nlatency = lnext -> second;
                auto lto = Latencies.find(to);
                auto latencyto = lto != Latencies.end() ? lto->second : 0;
                shift = nlatency-latencyto;
                nqueue.emplace(to, nl, nullptr, shift);
            }
        }
    }

    std::sort(Links.begin(), Links.end(), [] (const indk::LinkDefinition& l1, const indk::LinkDefinition& l2) {
        if (std::get<4>(l1) < std::get<4>(l2)) return true;
        return false;
    });

//    std::cout << std::endl;
//    for (auto l: Links) {
//        std::cerr << std::get<0>(l) << " -> " << std::get<1>(l) << " " << std::get<4>(l) << std::endl;
//    }

    PrepareID = id;
}

void indk::NeuralNet::doSyncNeuronStates(const std::string &name) {
    auto s = StateSyncList.find(name);
    if (s != StateSyncList.end()) {
        auto n1 = Neurons.find(s->first);
        if (n1 != Neurons.end()) {
            for (const auto& v: s->second) {
                auto n2 = Neurons.find(v);
                if (n2 != Neurons.end()) {
                    for (int i = 0; i < n1->second->getReceptorsCount(); i++) {
                        auto pos = n1 -> second -> getReceptor(i) -> getPosf();
                        n2 -> second -> getReceptor(i) -> getPosf() -> setPosition(pos);
                    }
                    n2 -> second -> setTime(n1->second->getTime());
                }
            }
        }
    }
}

void indk::NeuralNet::doStructurePrepare() {
    doParseLinks(Entries, "all");
}

/**
 * Send signals to neural network and get output signals.
 * @param Xx Input data vector that contain signals.
 * @return Output signals.
 */
std::vector<float> indk::NeuralNet::doSignalTransfer(const std::vector<std::vector<float>>& Xx, const std::string& ensemble) {
    if (indk::System::getComputeBackendKind() == -1) {
        if (indk::System::getVerbosityLevel() > 0)
            std::cerr << "Switching to default compute backend." << std::endl;

        indk::System::setComputeBackend(indk::System::ComputeBackends::Default);
    }
    std::vector<void*> v;
    std::vector<std::string> nsync;
    EntryList eentries;

    if (ensemble.empty()) {
        doParseLinks(Entries, "all");
        eentries = Entries;
    } else {
        auto ens = Ensembles.find(ensemble);
        if (ens != Ensembles.end()) {
            for (const auto &name: ens->second) {
                auto n = Neurons.find(name);
                nsync.push_back(name);
                auto nentries = n->second->getEntries();
                for (const auto &e: nentries) {
                    auto ne = doFindEntry(e);
                    if (ne != -1) {
                        eentries.emplace_back(Entries[ne]);
                    }
                }
            }
        }
        doParseLinks(eentries, ensemble);
    }

    switch (indk::System::getComputeBackendKind()) {
        case indk::System::ComputeBackends::Default:
            doReserveSignalBuffer(1);
            for (auto &X: Xx) {
                doSignalProcessStart({X}, eentries);
            }
            break;

        case indk::System::ComputeBackends::Multithread:
            if (getSignalBufferSize() != Xx.size()) doReserveSignalBuffer(Xx.size());
            for (const auto &n: Neurons) v.push_back((void*)n.second);
            indk::System::getComputeBackend() -> doRegisterHost(v);
            doSignalProcessStart(Xx, eentries);
            indk::System::getComputeBackend() -> doWaitTarget();
            indk::System::getComputeBackend() -> doUnregisterHost();
            break;

        case indk::System::ComputeBackends::OpenCL:
            if (getSignalBufferSize() != Xx.size()) doReserveSignalBuffer(Xx.size());
            for (const auto &n: Neurons) v.push_back((void*)n.second);
            indk::System::getComputeBackend() -> doRegisterHost(v);
            for (auto &X: Xx) {
                doSignalProcessStart({X}, eentries);
            }
            doSignalProcessStart({}, eentries);
            indk::System::getComputeBackend() -> doUnregisterHost();
            break;
    }

    LastUsedComputeBackend = indk::System::getComputeBackendKind();
    doInterlinkAppUpdateData();

    if (!ensemble.empty() && StateSyncEnabled) {
        for (const auto &name: nsync) {
            doSyncNeuronStates(name);
        }
    }

    return doSignalReceive();
}

/**
 * Send signals to neural network asynchronously.
 * @param Xx Input data vector that contain signals.
 * @param Callback Callback function for output signals.
 */
void indk::NeuralNet::doSignalTransferAsync(const std::vector<std::vector<float>>& Xx, const std::string& ensemble, const std::function<void(std::vector<float>)>& Callback) {
    std::function<void()> tCallback([this, Xx, ensemble, Callback] () {
        auto Y = doSignalTransfer(Xx, ensemble);

        if (Callback) {
            Callback(Y);
        }
    });
    std::thread CallbackThread(tCallback);
    CallbackThread.detach();
}

/**
 * Start neural network learning process.
 * @param Xx Input data vector that contain signals for learning.
 * @return Output signals.
 */
std::vector<float> indk::NeuralNet::doLearn(const std::vector<std::vector<float>>& Xx) {
    if (InterlinkService && InterlinkService->isInterlinked()) {
        InterlinkService -> doUpdateStructure(getStructure());
    }
    setLearned(false);
    doCreateNewScope();
    doPrepare();
    return doSignalTransfer(Xx);
}

/**
 * Recognize data by neural network.
 * @param Xx Input data vector that contain signals for recognizing.
 * @return Output signals.
 */
std::vector<float> indk::NeuralNet::doRecognise(const std::vector<std::vector<float>>& Xx, const std::string& ensemble) {
    setLearned(true);
    doPrepare();
    return doSignalTransfer(Xx, ensemble);
}

/**
 * Start neural network learning process asynchronously.
 * @param Xx Input data vector that contain signals for learning.
 * @param Callback Callback function for output signals.
 */
void indk::NeuralNet::doLearnAsync(const std::vector<std::vector<float>>& Xx, const std::function<void(std::vector<float>)>& Callback) {
    setLearned(false);
    doCreateNewScope();
    doPrepare();
    doSignalTransferAsync(Xx, "", Callback);
}

/**
 * Recognize data by neural network asynchronously.
 * @param Xx Input data vector that contain signals for recognizing.
 * @param Callback Callback function for output signals.
 */
void indk::NeuralNet::doRecogniseAsync(const std::vector<std::vector<float>>& Xx, const std::string& ensemble, const std::function<void(std::vector<float>)>& Callback) {
    setLearned(true);
    doPrepare();
    doSignalTransferAsync(Xx, ensemble, Callback);
}

/**
 * Get output signals.
 * @return Output signals vector.
 */
std::vector<float> indk::NeuralNet::doSignalReceive() {
    std::vector<float> ny;
    for (const auto& oname: Outputs) {
        auto n = Neurons.find(oname);
        if (n != Neurons.end()) ny.push_back(n->second->doSignalReceive().second);
    }
    return ny;
}

/**
 * Creates full copy of group of neurons.
 * @param From Source ensemble name.
 * @param To Name of new ensemble.
 * @param CopyEntries Copy entries during replication. So, if neuron `A1N1` (ensemble `A1`) has an entry `A1E1`
 * and you replicating to ensemble `A2`, a new entry `A2E1` will be added.
 */
void indk::NeuralNet::doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries) {
    json j;

    PrepareID = "";
    std::vector<std::string> enew;
    auto efrom = Ensembles.find(From);

    if (efrom != Ensembles.end()) {
        auto eto = Ensembles.find(To);
        auto lastname = efrom->second.back();

        std::map<std::string, std::string> newnames;
        for (auto &nn: efrom->second) {
            std::string nname;
            if (nn.substr(0, From.size()) == From) {
                nname = nn;
                nname.replace(0, From.size(), To);
            } else nname = To + nn;
            newnames.insert(std::make_pair(nn, nname));
        }

        for (auto &en: efrom->second) {
            json ji;

            auto n = Neurons.find(en);
            if (n != Neurons.end()) {
                auto nnew = new indk::Neuron(*n->second);
                std::string nname = newnames.find(en)->second;
                nnew -> setName(nname);

                auto entries = nnew -> getEntries();
                for (auto &e: entries) {
                    std::string ename = e;
                    auto r = newnames.find(e);
                    if (r != newnames.end()) {
                        nnew -> doReplaceEntryName(e, r->second);
                    } else if (CopyEntries) {
                        if (e.substr(0, From.size()) == From) {
                            ename = e;
                            ename.replace(0, From.size(), To);
                        } else ename = To + e;
                        nnew -> doReplaceEntryName(e, ename);
                    }

                    auto ne = doFindEntry(ename);
                    if (ne != -1) {
                        Entries[ne].second.push_back(nname);
                    } else if (CopyEntries && r == newnames.end()) {
                        std::vector<std::string> elinks;
                        elinks.push_back(nname);
                        j["entries"].push_back( ename);
                        Entries.emplace_back(ename, elinks);
                    }
                }

                auto outputlinks = nnew -> getLinkOutput();
                nnew -> doClearOutputLinks();
                for (auto &o: outputlinks) {
                    auto r = newnames.find(o);
                    if (r != newnames.end()) {
                        nnew -> doLinkOutput(r->second);
                    } else {
                        nnew -> doLinkOutput(o);
                    }
                }

                auto no = std::find(Outputs.begin(), Outputs.end(), en);
                if (no != Outputs.end()) {
                    Outputs.push_back(nname);
                    j["outputs"].push_back(nname);
                }

                auto nl = Latencies.find(en);
                if (nl != Latencies.end()) Latencies.insert(std::make_pair(nname, nl->second));

                Neurons.insert(std::make_pair(nname, nnew));

                entries = nnew -> getEntries();
                for (auto &e: entries) {
                    ji["input_signals"].push_back(e);
                }
                ji["old_name"] = en;
                ji["new_name"] = nname;
                j["neurons"].push_back(ji);

                auto sobject = StateSyncList.find(en);
                if (sobject == StateSyncList.end()) {
                    StateSyncList.insert(std::make_pair(en, std::vector<std::string>({nname})));
                } else {
                    sobject->second.push_back(nname);
                }

                if (eto == Ensembles.end()) {
                    enew.push_back(nname);
                } else {
                    eto->second.push_back(nname);
                }
            }
        }

        if (eto == Ensembles.end()) Ensembles.insert(std::make_pair(To, enew));
    }

    if (indk::System::getVerbosityLevel() > 1) {
        auto e = Ensembles.find(To);
        std::cout << "Entries: ";
        for (const auto& ne: Entries) {
            std::cout << ne.first << " ";
        }
        std::cout << std::endl;
        std::cout << e->first << " -";
        for (const auto& en: e->second) {
            std::cout << " " << en;
        }
        std::cout << std::endl;
        std::cout << "Outputs: ";
        for (const auto& o: Outputs) {
            std::cout << o << " ";
        }
        std::cout << std::endl;
    }
}

void indk::NeuralNet::doReserveSignalBuffer(int64_t L) {
    for (auto &n: Neurons) {
        n.second -> doReserveSignalBuffer(L);
    }
}

/**
 * Load neural network structure.
 * @param Stream Input stream of file that contains neural network structure in JSON format.
 */
void indk::NeuralNet::setStructure(std::ifstream &Stream) {
    if (!Stream.is_open()) {
        if (indk::System::getVerbosityLevel() > 0) std::cerr << "Error opening file" << std::endl;
        return;
    }
    std::string jstr;
    while (!Stream.eof()) {
        std::string rstr;
        getline(Stream, rstr);
        jstr.append(rstr);
    }
    setStructure(jstr);
}

/** \example samples/test/structure.json
 * Example of interference neural net structure. It can be used by NeuralNet class and indk::NeuralNet::setStructure method.
 */

/**
 * Load neural network structure.
 * @param Str JSON string that contains neural network structure.
 *
 * Format of neural network structure:
 * \code
 * {
 *      "entries": [<list of neural network entries>],
 *      "neurons": [{
 *          "name": <name of neuron>,
            "size": <size of neuron>,
            "dimensions": 3,
            "input_signals": [<list of input signals, it can be network entries or other neurons>],
            "ensemble": <the name of the ensemble to which the neuron will be connected>,
            "synapses": [{
                "entry": 0,
                "position": [100, 100, 100],
                "neurotransmitter": "activation",
                "k1": 1
            }],
            "receptors": [{
                "type": "cluster",
                "position": [100, 210, 100],
                "count": 15,
                "radius": 10
            }]
 *      }],
 *      "output_signals": [<list of output sources>],
 *      "name": "neural network structure name",
 *      "desc": "neural network structure description",
 *      "version": "neural network structure version"
 * }
 * \endcode
 *
 *
 * @note
 * Example of neural network structure can be found in the samples:
 * <a href="samples_2test_2structure_8json-example.html">test sample</a>,
 * <a href="samples_2test_2structure_8json-example.html">vision sample</a>
 *
 */
void indk::NeuralNet::setStructure(const std::string &Str) {
    for (const auto& N: Neurons) delete N.second;
    PrepareID = "";
    Entries.clear();
    Outputs.clear();
    Latencies.clear();
    Neurons.clear();
    Ensembles.clear();
    StateSyncList.clear();

    try {
        auto j = json::parse(Str);
        //std::cout << j.dump(4) << std::endl;
        Name = j["name"].get<std::string>();
        Description = j["desc"].get<std::string>();
        Version = j["version"].get<std::string>();

        std::multimap<std::string, std::string> links;
        for (auto &jneuron: j["neurons"].items()) {
            auto nname = jneuron.value()["name"].get<std::string>();
            for (auto &jinputs: jneuron.value()["input_signals"].items()) {
                auto iname = jinputs.value().get<std::string>();
                links.insert(std::make_pair(iname, nname));
            }
        }

        for (auto &jentry: j["entries"].items()) {
            auto ename = jentry.value().get<std::string>();
            std::vector<std::string> elinks;
            auto l = links.equal_range(ename);
            for (auto it = l.first; it != l.second; it++) {
                elinks.push_back(it->second);
                if (indk::System::getVerbosityLevel() > 1) std::cout << ename << " -> " << it->second << std::endl;
            }
            Entries.emplace_back(ename, elinks);
        }

        for (auto &joutput: j["output_signals"].items()) {
            auto oname = joutput.value().get<std::string>();
            if (indk::System::getVerbosityLevel() > 1) std::cout <<  "Output " << oname << std::endl;
            Outputs.push_back(oname);
        }

//        for (auto &l: links) {
//            std::cout << l.first << " - " << l.second << std::endl;
//        }

        for (auto &jneuron: j["neurons"].items()) {
            auto nname = jneuron.value()["name"].get<std::string>();
            auto nsize = jneuron.value()["size"].get<unsigned int>();
            auto ndimensions = jneuron.value()["dimensions"].get<unsigned int>();
            if (jneuron.value()["latency"] != nullptr) {
                auto nlatency = jneuron.value()["latency"].get<int>();
                if (indk::System::getVerbosityLevel() > 1) std::cout << nname << " with latency " << nlatency << std::endl;
                Latencies.insert(std::make_pair(nname, nlatency));
            }
            std::vector<std::string> nentries;
            for (auto &jent: jneuron.value()["input_signals"].items()) {
                nentries.push_back(jent.value().get<std::string>());
            }
            auto *N = new indk::Neuron(nsize, ndimensions, 0, nentries);

            if (jneuron.value()["ensemble"] != nullptr) {
                auto ename = jneuron.value()["ensemble"].get<std::string>();
                auto e = Ensembles.find(ename);
                if (e == Ensembles.end()) {
                    std::vector<std::string> en;
                    en.push_back(nname);
                    Ensembles.insert(std::make_pair(ename, en));
                } else {
                    e->second.push_back(nname);
                }
            }

            for (auto &jsynapse: jneuron.value()["synapses"].items()) {
                std::vector<float> pos;
                if (ndimensions != jsynapse.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jsynapse.value()["position"].items()) {
                    pos.push_back(jposition.value().get<float>());
                }
                float k1 = 1.2;
                if (jsynapse.value()["k1"] != nullptr) k1 = jsynapse.value()["k1"].get<float>();
                unsigned int tl = 0;
                if (jsynapse.value()["tl"] != nullptr) tl = jsynapse.value()["tl"].get<unsigned int>();
                int nt = 0;
                if (jsynapse.value()["neurotransmitter"] != nullptr) {
                    if (jsynapse.value()["neurotransmitter"].get<std::string>() == "deactivation")
                        nt = 1;
                }
                if (jsynapse.value()["type"] != nullptr && jsynapse.value()["type"].get<std::string>() == "cluster") {
                    auto sradius = jsynapse.value()["radius"].get<unsigned int>();
                    N -> doCreateNewSynapseCluster(pos, sradius, k1, tl, nt);
                } else {
                    auto sentryid = -1;
                    if (jsynapse.value()["entry"] != nullptr) {
                        sentryid = jsynapse.value()["entry"].get<unsigned int>();
                    } else {
                        std::cout << "Error: entry number must be set" << std::endl;
                        return;
                    }
                    auto sentry = jneuron.value()["input_signals"][sentryid];
                    N -> doCreateNewSynapse(sentry, pos, k1, tl, nt);
                }


            }
            for (auto &jreceptor: jneuron.value()["receptors"].items()) {
                std::vector<float> pos;
                if (ndimensions != jreceptor.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jreceptor.value()["position"].items()) {
                    pos.push_back(jposition.value().get<float>());
                }
                if (jreceptor.value()["type"] != nullptr && jreceptor.value()["type"].get<std::string>() == "cluster") {
                    auto rcount = jreceptor.value()["count"].get<unsigned int>();
                    auto rradius = jreceptor.value()["radius"].get<unsigned int>();
                    N -> doCreateNewReceptorCluster(pos, rradius, rcount);

                    for (auto i = N->getReceptorsCount()-rcount; i < N ->getReceptorsCount(); i++) {
                        auto r = N -> getReceptor(i);
                        for (auto &jscope: jreceptor.value()["scopes"].items()) {
                            pos.clear();
                            for (auto &jposition: jscope.value().items()) {
                                pos.push_back(jposition.value().get<float>());
                            }
                            N -> doCreateNewScope();
                            r -> setPos(new indk::Position(nsize, pos));
                        }
                    }
                } else {
                    N -> doCreateNewReceptor(pos);
                    for (auto &jscope: jreceptor.value()["scopes"].items()) {
                        pos.clear();
                        for (auto &jposition: jscope.value().items()) {
                            pos.push_back(jposition.value().get<float>());
                        }
                        auto r = N -> getReceptor(N->getReceptorsCount()-1);
                        r -> doCreateNewScope();
                        r -> setPos(new indk::Position(nsize, pos));
                    }
                }
            }

            auto l = links.equal_range(nname);
            for (auto it = l.first; it != l.second; it++) {
                N -> doLinkOutput(it->second);
                if (indk::System::getVerbosityLevel() > 1) std::cout << nname << " -> " << it->second << std::endl;
            }

            N -> setName(nname);
            Neurons.insert(std::make_pair(nname, N));
        }

        if (indk::System::getVerbosityLevel() > 1) {
            for (const auto &e: Ensembles) {
                std::cout << e.first << " -";
                for (const auto &en: e.second) {
                    std::cout << " " << en;
                }
                std::cout << std::endl;
            }
        }

        if (InterlinkService && InterlinkService->isInterlinked()) {
            InterlinkService -> setStructure(Str);
        }
    } catch (std::exception &e) {
        if (indk::System::getVerbosityLevel() > 0) std::cerr << "Error parsing structure: " << e.what() << std::endl;
    }
}

/**
 * Set neural network to `learned` state.
 * @param LearnedFlag
 */
void indk::NeuralNet::setLearned(bool LearnedFlag) {
    for (const auto& N: Neurons) {
        N.second -> setLearned(LearnedFlag);
    }
}

/**
 * Enable neuron state synchronization.
 * @param enable
 */
void indk::NeuralNet::setStateSyncEnabled(bool enabled) {
    StateSyncEnabled = enabled;
}

/**
 * Check if neural network is in learned state.
 * @return
 */
bool indk::NeuralNet::isLearned() {
    for (const auto& N: Neurons) {
        if (!N.second -> isLearned()) return false;
    }
    return true;
}

/**
 * Get neuron by name.
 * @param NName Neuron name.
 * @return indk::Neuron object pointer.
 */
indk::Neuron* indk::NeuralNet::getNeuron(const std::string& NName) {
    auto N = Neurons.find(NName);
    if (N != Neurons.end()) return N->second;
    return nullptr;
}

/**
 * Get count of neurons in neural network.
 * @return Count of neurons.
 */
uint64_t indk::NeuralNet::getNeuronCount() {
    return Neurons.size();
}

/**
 * Get neural network structure in JSON format.
 * @return JSON string that contains neural network structure.
 */
std::string indk::NeuralNet::getStructure(bool minimized) {
    json j;

    for (const auto& e: Entries) {
        j["entries"].push_back(e.first);
    }

    for (const auto& n: Neurons) {
        json jn;
        jn["name"] = n.second -> getName();
        jn["size"] = n.second -> getXm();
        jn["dimensions"] = n.second -> getDimensionsCount();

        auto nentries = n.second -> getEntries();
        for (const auto& ne: nentries) {
            jn["input_signals"].push_back(ne);
        }
        for (const auto& en: Ensembles) {
            for (const auto& nen: en.second) {
                if (nen == n.second->getName()) {
                    jn["ensemble"] = en.first;
                    break;
                }
            }
        }

        for (int i = 0; i < n.second->getEntriesCount(); i++) {
            auto ne = n.second -> getEntry(i);
            for (int s = 0; s < ne->getSynapsesCount(); s++) {
                auto ns = ne -> getSynapse(s);
                json js;
                js["entry"] = i;
                js["k1"] = ns -> getk1();

                switch (ns->getNeurotransmitterType()) {
                    case 0:
                        js["neurotransmitter"] = "activation";
                        break;
                    case 1:
                        js["neurotransmitter"] = "deactivation";
                        break;
                }

                for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                    js["position"].push_back(ns->getPos()->getPositionValue(p));
                }
                jn["synapses"].push_back(js);
            }
        }

        for (int r = 0; r < n.second->getReceptorsCount(); r++) {
            auto nr = n.second ->getReceptor(r);
            json jr;
            jr["type"] = "single";
            for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                jr["position"].push_back(nr->getPos0()->getPositionValue(p));
            }

            json jscopes;
            auto scopes = nr -> getReferencePosScopes();
            for (const auto &s: scopes) {
                json jscope;
                for (int p = 0; p < s->getDimensionsCount(); p++) {
                    jscope.push_back(s->getPositionValue(p));
                }
                jscopes.push_back(jscope);
            }
            if (!scopes.empty())
                jr["scopes"] = jscopes;

            jn["receptors"].push_back(jr);
        }

        auto l = Latencies.find(n.second->getName());
        if (l != Latencies.end()) {
            jn["latency"] = l->second;
        }

        j["neurons"].push_back(jn);
    }

    for (const auto& o: Outputs) {
        j["output_signals"].push_back(o);
    }

    j["name"] = Name;
    j["desc"] = Description;
    j["version"] = Version;

    if (indk::System::getVerbosityLevel() > 2) {
        std::cout << j.dump(4) << std::endl;
    }

    return minimized ? j.dump() : j.dump(4);
}

/**
 * Get neural network structure name.
 * @return String that contains name.
 */
std::string indk::NeuralNet::getName() {
    return Name;
}

/**
 * Get neural network structure description.
 * @return String that contains description.
 */
std::string indk::NeuralNet::getDescription() {
    return Description;
}

/**
 * Get neural network structure version.
 * @return String that contains version.
 */
std::string indk::NeuralNet::getVersion() {
    return Version;
}

/**
 * Get group of neurons by name.
 * @param ename Ensemble name.
 * @return Vector of indk::Neuron object pointers.
 */
std::vector<indk::Neuron*> indk::NeuralNet::getEnsemble(const std::string& ename) {
    auto e = Ensembles.find(ename);
    if (e != Ensembles.end()) {
        std::vector<indk::Neuron*> neurons;
        for (const auto& en: e->second) {
            neurons.push_back(getNeuron(en));
        }
        return neurons;
    }
    return {};
}

int64_t indk::NeuralNet::getSignalBufferSize() {
    int64_t size = -1;
    for (auto &n: Neurons) {
        auto nbsize = n.second -> getSignalBufferSize();
        if (size != -1 && nbsize >= size) continue;
        size = nbsize;
    }
    return size;
}

indk::NeuralNet::~NeuralNet() {
    for (const auto& N: Neurons) delete N.second;
}
