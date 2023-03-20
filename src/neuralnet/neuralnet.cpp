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
#include "../3rdparty/json.hpp"
#include "../../include/inn/neuralnet.h"
#include "../../include/inn/error.h"

typedef nlohmann::json json;
std::queue<std::tuple<std::string, std::string, void*, int64_t>> nqueue;

inn::NeuralNet::NeuralNet() {
    t = 0;
    Learned = false;

    if (inn::System::isSynchronizationNeeded()) {
        DataDoneEvent = new inn::Event();
    }
}

int64_t inn::NeuralNet::doFindEntry(const std::string& ename) {
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
std::vector<double> inn::NeuralNet::doComparePatterns() {
    std::vector<double> PDiffR, PDiffL, PDiff;
    for (const auto& O: Outputs) {
        auto n = Neurons.find(O);
        if (n == Neurons.end()) break;
        auto P = n -> second -> doComparePattern();
        PDiffR.push_back(std::get<0>(P));
        PDiffL.push_back(std::get<1>(P));
//        std::cout << O << " " << std::get<0>(P) << " " << std::get<1>(P) << std::endl;
    }
    double PDRMin = PDiffR[std::distance(PDiffR.begin(), std::min_element(PDiffR.begin(), PDiffR.end()))];
    double PDRMax = PDiffR[std::distance(PDiffR.begin(), std::max_element(PDiffR.begin(), PDiffR.end()))] - PDRMin;

    double PDLMin = PDiffL[std::distance(PDiffL.begin(), std::min_element(PDiffL.begin(), PDiffL.end()))];
    double PDLMax = PDiffL[std::distance(PDiffL.begin(), std::max_element(PDiffL.begin(), PDiffL.end()))] - PDLMin;
//    std::cout << PDRMin << " " << PDRMax << " " << PDLMin << " " << PDLMax << std::endl;
    for (auto &PDR: PDiffR) {
        //PDR = 1 - (PDR-PDRMin) / PDRMax;
        if (PDRMax != 0) PDiff.push_back(1 - (PDR-PDRMin) / PDRMax);
        else PDiff.push_back(1);
    }
//    for (auto &PDL: PDiffL) {
//        PDL = 1 - (PDL-PDLMin) / PDLMax;
//        std::cout << PDL << std::endl;
//    }
//    for (auto i = 0; i < Outputs.size(); i++) {
//        PDiff.push_back((PDiffR[i]+PDiffL[i])/2);
//    }
    return PDiff;
}

/**
 * Resets all neurons in the network.
 * See inn::Neuron::doReset() method for details.
 */
void inn::NeuralNet::doReset() {
    t = 0;
    for (const auto& N: Neurons) N.second -> doReset();
}

void inn::NeuralNet::doSignalProcessStart() {
    while (!nqueue.empty()) {
        auto i = nqueue.front();
        nqueue.pop();

        auto from = std::get<0>(i);
        auto to = std::get<1>(i);
        auto object = std::get<2>(i);
        auto time = std::get<3>(i);

        auto lfrom = Latencies.find(from);
        auto latencyfrom = lfrom != Latencies.end() ? lfrom->second : 0;
        auto lto = Latencies.find(to);
        auto latencyto = lto != Latencies.end() ? lto->second : 0;

        auto n = Neurons.find(to);

        if (n != Neurons.end()) {
            if (time == 0) {
                auto waiting = n -> second -> getWaitingEntries();
                for (auto &we: waiting) {
                    auto nfrom = Latencies.find(we);
                    if (nfrom != Latencies.end() && nfrom->second > latencyto) {
                        n -> second -> doSignalSendEntry(we, 0, time);
                    }
                }
            }

            double value = 0;
            auto eid = doFindEntry(from);
            if (eid >= 0) {
                value = *(double*)object;
                delete (double*)object;
            } else {
                auto nprev = (inn::Neuron*)object;

                auto shift = latencyfrom > latencyto;
                if (nprev->getState(time-shift) != inn::Neuron::States::Computed) {
                    nqueue.push(i);
                    continue;
                }
                value = nprev -> doSignalReceive(time-shift).second;
            }

            bool done = n -> second -> doSignalSendEntry(from, value, time);

            if (done) {
                auto nlinks = n -> second -> getLinkOutput();
                for (auto &nl: nlinks) {
                    auto shift = 0;
                    auto nlatency = 0;
                    auto lnext = Latencies.find(nl);
                    if (lnext != Latencies.end()) nlatency = lnext -> second;
                    if (latencyto > nlatency) shift = 1;
                    nqueue.emplace(to, nl, n->second, time+shift);
                }
            }
        }
    }
}

/**
 * Send signals to neural network.
 * @param X Input data vector that contain signals for current time.
 */
void inn::NeuralNet::doSignalSend(const std::vector<double>& X) {
    if (inn::System::getComputeBackendKind() == -1) inn::System::setComputeBackend(inn::System::ComputeBackends::Default);
    int xi = 0;

    for (auto &e: Entries) {
        for (auto &en: e.second) {
            nqueue.emplace(e.first, en, new double(X[xi]), t);
        }
        xi++;
    }

    if (!inn::System::isSynchronizationNeeded()) {
        doSignalProcessStart();
    }

    t++;
}

/**
 * Send signals to neural network and get output signals.
 * @param Xx Input data vector that contain signals.
 * @return Output signals.
 */
std::vector<double> inn::NeuralNet::doSignalTransfer(const std::vector<std::vector<double>>& Xx) {
    if (!inn::System::isSynchronizationNeeded()) {
        doReserveSignalBuffer(1);
    }

    for (auto &X: Xx) {
        doSignalSend(X);
    }

    if (inn::System::isSynchronizationNeeded()) {
        if (getSignalBufferSize() != Xx.size()) doReserveSignalBuffer(Xx.size());
        std::vector<void*> v;
        for (const auto &n: Neurons) v.push_back((void*)n.second);
        inn::System::getComputeBackend() -> doRegisterHost(v);
        doSignalProcessStart();
        inn::System::getComputeBackend() -> doWaitTarget();
    }

    return doSignalReceive();
}

/**
 * Send signals to neural network asynchronously.
 * @param Xx Input data vector that contain signals.
 * @param Callback Callback function for output signals.
 */
void inn::NeuralNet::doSignalTransferAsync(const std::vector<std::vector<double>>& Xx, const std::function<void(std::vector<double>)>& Callback) {
    t = 0;

    std::function<void()> tCallback([this, Xx, Callback] () {
        for (auto &X: Xx) {
            doSignalSend(X);
        }

        if (inn::System::isSynchronizationNeeded()) {
            doSignalProcessStart();
        }

        if (Callback) {
            auto Y = doSignalReceive();
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
std::vector<double> inn::NeuralNet::doLearn(const std::vector<std::vector<double>>& Xx) {
    setLearned(false);
    doReset();
    return doSignalTransfer(Xx);
}

/**
 * Recognize data by neural network.
 * @param Xx Input data vector that contain signals for recognizing.
 * @return Output signals.
 */
std::vector<double> inn::NeuralNet::doRecognise(const std::vector<std::vector<double>>& Xx) {
    setLearned(true);
    doReset();
    return doSignalTransfer(Xx);
}

/**
 * Start neural network learning process asynchronously.
 * @param Xx Input data vector that contain signals for learning.
 * @param Callback Callback function for output signals.
 */
void inn::NeuralNet::doLearnAsync(const std::vector<std::vector<double>>& Xx, const std::function<void(std::vector<double>)>& Callback) {
    setLearned(false);
    doReset();
    doSignalTransferAsync(Xx, Callback);
}

/**
 * Recognize data by neural network asynchronously.
 * @param Xx Input data vector that contain signals for recognizing.
 * @param Callback Callback function for output signals.
 */
void inn::NeuralNet::doRecogniseAsync(const std::vector<std::vector<double>>& Xx, const std::function<void(std::vector<double>)>& Callback) {
    setLearned(true);
    doReset();
    doSignalTransferAsync(Xx, Callback);
}

/**
 * Get output signals.
 * @return Output signals vector.
 */
std::vector<double> inn::NeuralNet::doSignalReceive() {
    std::vector<double> ny;
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
void inn::NeuralNet::doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries) {
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
            auto n = Neurons.find(en);
            if (n != Neurons.end()) {
                auto nnew = new inn::Neuron(*n->second);
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
                if (no != Outputs.end()) Outputs.push_back(nname);

                auto nl = Latencies.find(en);
                if (nl != Latencies.end()) Latencies.insert(std::make_pair(nname, nl->second));

                Neurons.insert(std::make_pair(nname, nnew));

                if (eto == Ensembles.end()) {
                    enew.push_back(nname);
                } else {
                    eto->second.push_back(nname);
                }
            }
        }

        if (eto == Ensembles.end()) Ensembles.insert(std::make_pair(To, enew));
    }

    if (inn::System::getVerbosityLevel() > 1) {
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

void inn::NeuralNet::doReserveSignalBuffer(int64_t L) {
    for (auto &n: Neurons) {
        n.second -> doReserveSignalBuffer(L);
    }
}

/**
 * Load neural network structure.
 * @param Stream Input stream of file that contains neural network structure in JSON format.
 */
void inn::NeuralNet::setStructure(std::ifstream &Stream) {
    if (!Stream.is_open()) {
        if (inn::System::getVerbosityLevel() > 0) std::cerr << "Error opening file" << std::endl;
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
 * Example of interference neural net structure. It can be used by NeuralNet class and inn::NeuralNet::setStructure method.
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
void inn::NeuralNet::setStructure(const std::string &Str) {
    Entries.clear();
    Outputs.clear();
    Latencies.clear();
    Neurons.clear();
    Ensembles.clear();

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
                if (inn::System::getVerbosityLevel() > 1) std::cout << ename << " -> " << it->second << std::endl;
            }
            Entries.emplace_back(ename, elinks);
        }

        for (auto &joutput: j["output_signals"].items()) {
            auto oname = joutput.value().get<std::string>();
            if (inn::System::getVerbosityLevel() > 1) std::cout <<  "Output " << oname << std::endl;
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
                if (inn::System::getVerbosityLevel() > 1) std::cout << nname << " with latency " << nlatency << std::endl;
                Latencies.insert(std::make_pair(nname, nlatency));
            }
            std::vector<std::string> nentries;
            for (auto &jent: jneuron.value()["input_signals"].items()) {
                nentries.push_back(jent.value().get<std::string>());
            }
            auto *N = new inn::Neuron(nsize, ndimensions, 0, nentries);

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
                std::vector<double> pos;
                if (ndimensions != jsynapse.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jsynapse.value()["position"].items()) {
                    pos.push_back(jposition.value().get<double>());
                }
                double k1 = 1.2;
                if (jsynapse.value()["k1"] != nullptr) k1 = jsynapse.value()["k1"].get<double>();
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
                std::vector<double> pos;
                if (ndimensions != jreceptor.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jreceptor.value()["position"].items()) {
                    pos.push_back(jposition.value().get<double>());
                }
                if (jreceptor.value()["type"] != nullptr && jreceptor.value()["type"].get<std::string>() == "cluster") {
                    auto rcount = jreceptor.value()["count"].get<unsigned int>();
                    auto rradius = jreceptor.value()["radius"].get<unsigned int>();
                    N -> doCreateNewReceptorCluster(pos, rradius, rcount);
                } else {
                    N -> doCreateNewReceptor(pos);
                }
            }

            auto l = links.equal_range(nname);
            for (auto it = l.first; it != l.second; it++) {
                N -> doLinkOutput(it->second);
                if (inn::System::getVerbosityLevel() > 1) std::cout << nname << " -> " << it->second << std::endl;
            }

            N -> setName(nname);
            Neurons.insert(std::make_pair(nname, N));
        }

        if (inn::System::getVerbosityLevel() > 1) {
            for (const auto &e: Ensembles) {
                std::cout << e.first << " -";
                for (const auto &en: e.second) {
                    std::cout << " " << en;
                }
                std::cout << std::endl;
            }
        }
    } catch (std::exception &e) {
        if (inn::System::getVerbosityLevel() > 0) std::cerr << "Error parsing structure: " << e.what() << std::endl;
    }
}

/**
 * Set neural network to `learned` state.
 * @param LearnedFlag
 */
void inn::NeuralNet::setLearned(bool LearnedFlag) {
    for (const auto& N: Neurons) {
        N.second -> setLearned(LearnedFlag);
    }
}

/**
 * Check if neural network is in learned state.
 * @return
 */
bool inn::NeuralNet::isLearned() {
    for (const auto& N: Neurons) {
        if (!N.second -> isLearned()) return false;
    }
    return true;
}

/**
 * Get neuron by name.
 * @param NName Neuron name.
 * @return inn::Neuron object pointer.
 */
inn::Neuron* inn::NeuralNet::getNeuron(const std::string& NName) {
    auto N = Neurons.find(NName);
    if (N != Neurons.end()) return N->second;
    return nullptr;
}

/**
 * Get count of neurons in neural network.
 * @return Count of neurons.
 */
uint64_t inn::NeuralNet::getNeuronCount() {
    return Neurons.size();
}

/**
 * Get neural network structure in JSON format.
 * @return JSON string that contains neural network structure.
 */
std::string inn::NeuralNet::getStructure() {
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
            if (nr->isLocked()) {
                for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                    jr["learned_position"].push_back(nr->getPos()->getPositionValue(p));
                }
            }

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

    if (inn::System::getVerbosityLevel() > 2) {
        std::cout << j.dump(4) << std::endl;
    }

    return j.dump();
}

/**
 * Get neural network structure name.
 * @return String that contains name.
 */
std::string inn::NeuralNet::getName() {
    return Name;
}

/**
 * Get neural network structure description.
 * @return String that contains description.
 */
std::string inn::NeuralNet::getDescription() {
    return Description;
}

/**
 * Get neural network structure version.
 * @return String that contains version.
 */
std::string inn::NeuralNet::getVersion() {
    return Version;
}

/**
 * Get group of neurons by name.
 * @param ename Ensemble name.
 * @return Vector of inn::Neuron object pointers.
 */
std::vector<inn::Neuron*> inn::NeuralNet::getEnsemble(const std::string& ename) {
    auto e = Ensembles.find(ename);
    if (e != Ensembles.end()) {
        std::vector<inn::Neuron*> neurons;
        for (const auto& en: e->second) {
            neurons.push_back(getNeuron(en));
        }
        return neurons;
    }
    return {};
}

int64_t inn::NeuralNet::getSignalBufferSize() {
    int64_t size = -1;
    for (auto &n: Neurons) {
        auto nbsize = n.second -> getSignalBufferSize();
        if (size != -1 && nbsize >= size) continue;
        size = nbsize;
    }
    return size;
}

inn::NeuralNet::~NeuralNet() {
    for (const auto& N: Neurons) delete N.second;
}
