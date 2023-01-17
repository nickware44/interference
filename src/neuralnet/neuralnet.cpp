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

inn::NeuralNet::NeuralNet() {
    t = 0;
    Learned = false;

    if (inn::System::isSynchronizationNeeded()) {
        DataDoneEvent = new inn::Event();
    }
}

/**
 * Compare neuron patterns (learning and recognition patterns) for all output neurons.
 * @return Vector of pattern difference values for each output neuron.
 */
std::vector<double> inn::NeuralNet::doComparePatterns() {
    std::vector<double> PDiffR, PDiffL, PDiff;
    for (auto O: Outputs) {
//        auto P = O -> doComparePattern();
//        PDiffR.push_back(std::get<0>(P));
//        PDiffL.push_back(std::get<1>(P));
    }
    double PDRMin = PDiffR[std::distance(PDiffR.begin(), std::min_element(PDiffR.begin(), PDiffR.end()))];
    double PDRMax = PDiffR[std::distance(PDiffR.begin(), std::max_element(PDiffR.begin(), PDiffR.end()))] - PDRMin;

    double PDLMin = PDiffL[std::distance(PDiffL.begin(), std::min_element(PDiffL.begin(), PDiffL.end()))];
    double PDLMax = PDiffL[std::distance(PDiffL.begin(), std::max_element(PDiffL.begin(), PDiffL.end()))] - PDLMin;

//    for (auto &PDR: PDiffR) {
//        PDR = 1 - (PDR-PDRMin) / PDRMax;
//    }
    for (auto &PDL: PDiffL) {
        PDL = 1 - (PDL-PDLMin) / PDLMax;
        std::cout << PDL << std::endl;
    }
    for (auto i = 0; i < Outputs.size(); i++) {
        PDiff.push_back((PDiffR[i]+PDiffL[i])/2);
    }
    return PDiffR;
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
    int64_t begin = 0, end = 1;

    while (begin < ContextCascade.size()) {

        for (auto i = begin; i < end && i < ContextCascade.size(); i++) {
            auto &context = ContextCascade[i];
            auto &queue = context.first;
            auto &pending = context.second;

            if (inn::System::isSynchronizationNeeded() && !pending.empty()) {
                std::vector<std::string> pending_n = pending;
                pending.clear();
                while (!pending_n.empty()) {
                    auto p = pending_n.back();
                    pending_n.pop_back();
                    auto ne = Neurons.find(p);
                    if (ne->second->getState() == inn::Neuron::States::Computed) {
                        if (i == end-1) {
                            end++;
                        }
                        auto nlinks = ne -> second -> getLinkOutput();
                        for (auto &nl: nlinks) {
                            auto nr = ne->second->doSignalReceive().second;
                            auto nt = ne->second->getTime();
                            queue.push(std::make_tuple(p, nl, nr, nt));
                            if (inn::System::getVerbosityLevel() > 3) {
                                std::cout << "(" << ne->second->getTime() << ", " << i << ") " << "Added "
                                          << p << " -> " << nl << " (" << ne->second->doSignalReceive().second << ")"
                                          << std::endl;
                            }
                        }
                    } else pending.push_back(p);
                }
            }

            while (!queue.empty()) {
                auto e = queue.front();

                auto n = Neurons.find(std::get<1>(e));
                if (n == Neurons.end()) break;
                auto nf = Neurons.find(std::get<0>(e));
                auto tt = std::get<3>(e);

                if (n->second->getState() == inn::Neuron::States::Pending || (n->second->getState() == inn::Neuron::States::Computed && !n->second->getLinkOutput().empty()) ||
                        (nf != Neurons.end() && !(tt == n->second->getTime()+1 || tt == n->second->getTime()))) {

                    if (inn::System::getVerbosityLevel() > 3) {
                        std::cout << "Queue size " << queue.size() << std::endl;
                        std::cout << "Waiting " << std::get<0>(e) << " " << std::get<1>(e) <<
                                  " " << (nf != Neurons.end() ? nf->second->getTime() : 0) << " "
                                  << n->second->getTime() << " (" << tt << ", " << i << ") " << n->second->getState()
                                  << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }

                    break;
                }

                queue.pop();

                if (inn::System::getVerbosityLevel() > 2) {
                    std::cout << "(" << tt << ", " << i << ") " << std::get<0>(e) << " -> "
                              << std::get<1>(e) << " (" << (nf != Neurons.end() ? nf->second->getTime() : 0) << "-"
                              << n->second->getTime()
                              << " " << std::get<2>(e) << ")" << std::endl;
                }

                if (tt == 1) {
                    auto waiting = n -> second -> getWaitingEntries();
                    auto l = Latencies.find(std::get<1>(e));
                    auto latency = l == Latencies.end() ? 0 : l->second;
                    for (auto &we: waiting) {
                        auto nfrom = Latencies.find(we);
                        if (nfrom != Latencies.end() && nfrom->second > latency && std::get<0>(e) != we) {
                            n -> second -> doSignalSendEntry(we, 0, {});
                        }
                    }
                }

                auto done = n -> second -> doSignalSendEntry(std::get<0>(e), std::get<2>(e), {});

                if (done) {
                    if (n->second->getState() == inn::Neuron::States::Pending) {
                        pending.push_back(std::get<1>(e));
                        continue;
                    }
                    auto nlinks = n -> second -> getLinkOutput();
                    for (auto &nl: nlinks) {
                        queue.push(std::make_tuple(std::get<1>(e), nl, n->second->doSignalReceive().second, std::get<3>(e)));
                    }

                    if (i == end-1) {
                        end++;
                    }
                }
            }

            if (queue.empty() && pending.empty() && i == begin && begin+1 < end) {
                begin++;
            }
        }
    }
    ContextCascade.clear();
}

/**
 * Send signals to neural network.
 * @param X Input data vector that contain signals for current time.
 */
void inn::NeuralNet::doSignalSend(const std::vector<double>& X) {
    t++;
    int xi = 0;
    std::queue<std::tuple<std::string, std::string, double, int64_t>> q;
    std::vector<std::string> p;

    for (auto &e: Entries) {
        for (auto &en: e.second) {
            q.push(std::make_tuple(e.first, en, X[xi], t));
        }
        xi++;
    }

    ContextCascade.emplace_back(q, p);

    if (!inn::System::isSynchronizationNeeded()) {
        doSignalProcessStart();
    }
}

/**
 * Send signals to neural network and get output signals.
 * @param Xx Input data vector that contain signals.
 * @return Output signals.
 */
std::vector<double> inn::NeuralNet::doSignalTransfer(const std::vector<std::vector<double>>& Xx) {

    for (auto &X: Xx) {
        doSignalSend(X);
    }

    if (inn::System::isSynchronizationNeeded()) {
        doSignalProcessStart();
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
 */
void inn::NeuralNet::doReplicateEnsemble(const std::string& From, const std::string& To) {
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
                    auto r = newnames.find(e);
                    if (r != newnames.end()) {
                        nnew -> doReplaceEntryName(e, r->second);
                    }
                    auto ne = Entries.find(e);
                    if (ne != Entries.end()) {
                        ne->second.push_back(nname);
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
            Entries.insert(std::make_pair(ename, elinks));
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
                auto sentryid = jsynapse.value()["entry"].get<unsigned int>();
                auto sentry = jneuron.value()["input_signals"][sentryid];
                int nt = 0;
                if (jsynapse.value()["neurotransmitter"] != nullptr) {
                    if (jsynapse.value()["neurotransmitter"].get<std::string>() == "deactivation")
                        nt = 1;
                }
                N -> doCreateNewSynapse(sentry, pos, k1, tl, nt);
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
                    auto rradius = jreceptor.value()["count"].get<unsigned int>();
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
//    auto j = json::parse(R"({"entries": [],
//                                            "neurons": [],
//                                            "output_signals": [],
//                                            "name": "",
//                                            "desc": "",
//                                            "version": ""})");

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

        for (const auto& ne: nentries) {
            jn["input_signals"].push_back(ne);
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

    std::cout << j.dump(4) << std::endl;

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

inn::NeuralNet::~NeuralNet() {
    for (const auto& N: Neurons) delete N.second;
}
