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

    if (inn::isSynchronizationNeeded()) {
        DataDoneEvent = new inn::Event();
    }
}

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

            if (inn::isSynchronizationNeeded() && !pending.empty()) {
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
                            if (inn::getVerbosityLevel() > 3) {
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

                    if (inn::getVerbosityLevel() > 3) {
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

                if (inn::getVerbosityLevel() > 2) {
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

    if (!inn::isSynchronizationNeeded()) {
        doSignalProcessStart();
    }
}

std::vector<double> inn::NeuralNet::doSignalTransfer(const std::vector<std::vector<double>>& Xx) {
    t = 0;

    for (auto &X: Xx) {
        doSignalSend(X);
    }

    if (inn::isSynchronizationNeeded()) {
        doSignalProcessStart();
    }

    return doSignalReceive();
}

void inn::NeuralNet::doSignalTransferAsync(const std::vector<std::vector<double>>& Xx, const std::function<void(std::vector<double>)> Callback) {
    t = 0;

    std::function<void()> tCallback([this, Xx, Callback] () {
        for (auto &X: Xx) {
            doSignalSend(X);
        }

        if (inn::isSynchronizationNeeded()) {
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

std::vector<double> inn::NeuralNet::doSignalReceive() {
    std::vector<double> ny;
    for (const auto& oname: Outputs) {
        auto n = Neurons.find(oname);
        if (n != Neurons.end()) ny.push_back(n->second->doSignalReceive().second);
    }
    return ny;
}

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

    if (inn::getVerbosityLevel() > 1) {
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

inn::Neuron* inn::NeuralNet::getNeuron(const std::string& NName) {
    auto N = Neurons.find(NName);
    if (N != Neurons.end()) return N->second;
    return nullptr;
}

uint64_t inn::NeuralNet::getNeuronCount() {
    return Neurons.size();
}

void inn::NeuralNet::setStructure(std::ifstream &Stream) {
    if (!Stream.is_open()) {
        if (inn::getVerbosityLevel() > 0) std::cerr << "Error opening file" << std::endl;
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
                if (inn::getVerbosityLevel() > 1) std::cout << ename << " -> " << it->second << std::endl;
            }
            Entries.insert(std::make_pair(ename, elinks));
        }

        for (auto &joutput: j["output_signals"].items()) {
            auto oname = joutput.value().get<std::string>();
            if (inn::getVerbosityLevel() > 1) std::cout <<  "Output " << oname << std::endl;
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
                if (inn::getVerbosityLevel() > 1) std::cout << nname << " with latency " << nlatency << std::endl;
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

                N -> doCreateNewSynapse(sentry, pos, k1, tl);
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
                if (inn::getVerbosityLevel() > 1) std::cout << nname << " -> " << it->second << std::endl;
            }

            N -> setName(nname);
            Neurons.insert(std::make_pair(nname, N));
        }

        if (inn::getVerbosityLevel() > 1) {
            for (const auto &e: Ensembles) {
                std::cout << e.first << " -";
                for (const auto &en: e.second) {
                    std::cout << " " << en;
                }
                std::cout << std::endl;
            }
        }
    } catch (std::exception &e) {
        if (inn::getVerbosityLevel() > 0) std::cerr << "Error parsing structure: " << e.what() << std::endl;
    }
}

std::string inn::NeuralNet::getStructure() {
    return {};
}

std::string inn::NeuralNet::getName() {
    return Name;
}

std::string inn::NeuralNet::getDescription() {
    return Description;
}

std::string inn::NeuralNet::getVersion() {
    return Version;
}

inn::NeuralNet::~NeuralNet() {
    for (const auto& N: Neurons) delete N.second;
}
