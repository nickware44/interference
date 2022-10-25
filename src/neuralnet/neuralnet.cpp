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
std::queue<std::tuple<std::string, std::string, double, int64_t>> nqueue;
std::map<std::string, inn::Neuron*> Neurons;
std::map<std::string, int> Latencies;
inn::Event *DataDoneEvent;
std::mutex m;

inn::NeuralNet::NeuralNet() {
    EntriesCount = 0;
    LDRCounterE = 0;
    LDRCounterN = 0;
    t = 0;
    DataDone = false;
    Learned = false;

    if (inn::isSynchronizationNeeded()) {
        DataDoneEvent = new inn::Event();
        doSignalProcessStart();
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

void inn::NeuralNet::doPrepare() {
//    std::sort(Neurons.begin(), Neurons.end(), [](const inn::NeuralNet::NeuronDefinition &N1, const inn::NeuralNet::NeuronDefinition &N2) -> bool
//    {
//        return std::get<1>(N1) > std::get<1>(N2);
//    });
    for (const auto& N: Neurons) N.second -> doPrepare();
}

void inn::NeuralNet::doFinalize() {
    if (Neurons.empty()) return;
    /*
    std::vector<double> nX;
    unsigned int Tl = std::get<1>(Neurons[0]);
    if (Tl) {
        for (auto i = 0; i < EntriesCount; i++) nX.push_back(0);
        for (auto i = 0; i < Tl; i++) doSignalSend(nX);
        DataDone = true;
        while (true) {
            bool End = true;
            for (auto N: Neurons) {
                if (std::get<1>(N) == Tl) {
                    LinkMapRange R = NeuronLinks.equal_range(std::get<2>(N));
                    for (auto it = R.first; it != R.second; ++it) {
                        //std::cout << it->second.getTime() << std::endl;
                        if ((!Learned && it->second.getTime() != t) || (Learned && it->second.getTime() != t+it->second.getLinkFromE()->getTlo())) {
                            End = false;
                            break;
                        }
                    }
                    if (!End) break;
                }
            }
            if (End) break;
            doSignalSend(nX);
        }
    }
    for (auto N: Neurons) std::get<2>(N) -> doFinalize();
    Learned = true;
     */
}

void inn::NeuralNet::doReset() {
    t = 0;
    DataDone = false;
    //for (auto &NL: NeuronLinks) NL.second.doResetSignalController();
    for (const auto& N: Neurons) N.second -> doReinit();
}

void inn::NeuralNet::doSignalProcessStart() {
    std::function<void()> tWatcher([] () {
        std::vector<std::string> pending;

        while (!nqueue.empty() || inn::isSynchronizationNeeded()) {
            if (inn::isSynchronizationNeeded() && nqueue.empty() && pending.empty()) {
                DataDoneEvent -> doNotifyOne();
                inn::doNeuralNetSyncWait();
            }

            if (inn::isSynchronizationNeeded()) std::lock_guard<std::mutex> guard(m);

            if (inn::isSynchronizationNeeded()) {
                std::vector<std::string> pending_n;
                while (!pending.empty()) {
                    auto p = pending.back();
                    pending.pop_back();
                    auto ne = Neurons.find(p);
                    if (!ne->second->isPending()) {
                        auto nlinks = ne -> second -> getLinkOutput();
                        for (auto &nl: nlinks) {
                            auto nr = ne->second->doSignalReceive().second;
                            auto nt = ne->second->getTime();
                            nqueue.push(std::make_tuple(p, nl, nr, nt));
//                            std::cout  << "(" << ne->second->getTime() << ") " << "Added "
//                                << p << " -> " << nl << " (" << ne->second->doSignalReceive().second << ")" << std::endl;
                        }
                        //inn::doNeuralNetSyncWait();
                    } else pending_n.push_back(p);
                }
                pending = std::move(pending_n);
            }

            if (nqueue.empty()) continue;

            auto i = nqueue.front();
            nqueue.pop();
            auto tt = std::get<3>(i);
            auto n = Neurons.find(std::get<1>(i));
            auto nf = Neurons.find(std::get<0>(i));
            auto pn = std::find(pending.begin(), pending.end(), std::get<1>(i));
            if (n != Neurons.end() && (n->second->isPending() || pn != pending.end() ||
//                    (nf != Neurons.end() && !(nf->second->getTime() == n->second->getTime()+1 || nf->second->getTime() == n->second->getTime())))) {
                    (nf != Neurons.end() && !(tt == n->second->getTime()+1 || tt == n->second->getTime())))) {
#if 0
                std::cout << "Queue size " << nqueue.size() << std::endl;
                std::cout << "Resending to queue " << std::get<0>(i) << " " << std::get<1>(i) <<
                    " " << (nf != Neurons.end()?nf->second->getTime():0) << " " << n->second->getTime() << " " << tt << n->second->isPending() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
                nqueue.push(i);
                continue;
            }

            if (n != Neurons.end()) {
                bool done = false;

//                std::cout << "(" << tt << ") " << std::get<0>(i) << " -> "
//                          << std::get<1>(i) << " (" << (nf!=Neurons.end()?nf->second->getTime():0) << "-" <<  n->second->getTime()
//                          << " " << std::get<2>(i) << ")" << std::endl;

                if (tt == 1) {
                    auto waiting = n -> second -> getWaitingEntries();
                    auto l = Latencies.find(std::get<1>(i));
                    auto latency = l == Latencies.end() ? 0 : l->second;
                    for (auto &we: waiting) {
                        auto nfrom = Latencies.find(we);
                        if (nfrom != Latencies.end() && nfrom->second > latency && std::get<0>(i) != we) {
//                            std::cout << we << " (latency " << nfrom->second << ") -> " << std::get<1>(i) << " (0)" << std::endl;
                            n -> second -> doSignalSendEntry(we, 0, {});
                        }
                    }
                }

                //if (n->second->getTime() != tt) {
//                    std::cout << "(" << tt << ") " << "Sending signal to " << std::get<1>(i) << std::endl;
                done = n -> second -> doSignalSendEntry(std::get<0>(i), std::get<2>(i), {});
                //}

                if (done) {
                    if (n->second->isPending()) {
                        pending.push_back(std::get<1>(i));
                        continue;
                    }
//                    std::cout << "(" << tt << ") " << std::get<1>(i) << " computed" << std::endl;
                    auto nlinks = n -> second -> getLinkOutput();
                    for (auto &nl: nlinks) {
                        nqueue.push(std::make_tuple(std::get<1>(i), nl, n->second->doSignalReceive().second, std::get<3>(i)));
                    }
                }
//                std::cout << "queue " << nqueue.size() << std::endl;
            }
        }
    });

    if (inn::isSynchronizationNeeded()) {
        std::thread Worker(tWatcher);
        Worker.detach();
    } else {
        tWatcher();
    }
}

void inn::NeuralNet::doSignalSend(const std::vector<double>& X) {
    t++;
    if (inn::isSynchronizationNeeded()) std::lock_guard<std::mutex> guard(m);
    int xi = 0;
    for (auto &e: Entries) {
        for (auto &en: e.second) {
            nqueue.push(std::make_tuple(e.first, en, X[xi], t));
        }
        xi++;
    }

    if (!inn::isSynchronizationNeeded()) {
        doSignalProcessStart();
    } else {
//        doNeuralNetSync();
    }
}

std::vector<double> inn::NeuralNet::doSignalTransfer(const std::vector<std::vector<double>>& Xx) {
    t = 0;
    for (auto &X: Xx) {
        doSignalSend(X);
    }

    if (inn::isSynchronizationNeeded()) {
        doNeuralNetSync();
        DataDoneEvent -> doWait();
    } else {
//        doSignalProcessStart();
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
            doNeuralNetSync();
        }

        DataDoneEvent -> doWait();
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

//                for (auto &e: nnew->getEntries()) {
//                    std::cout << e << std::endl;
//                }
//
//                for (auto &o: nnew->getLinkOutput()) {
//                    std::cout << o << std::endl;
//                }

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
//    for (const auto& e: Ensembles) {
//        std::cout << e.first << " -";
//        for (const auto& en: e.second) {
//            std::cout << " " << en;
//        }
//        std::cout << std::endl;
//    }
//    for (const auto& o: Outputs) {
//        std::cout << o << " ";
//    }
//    std::cout << std::endl;
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
        std::cout << "Error opening file" << std::endl;
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
                std::cout << ename << " -> " << it->second << std::endl;
            }
            Entries.insert(std::make_pair(ename, elinks));
        }

        for (auto &joutput: j["output_signals"].items()) {
            auto oname = joutput.value().get<std::string>();
            std::cout <<  "Output " << oname << std::endl;
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
                std::cout << nname << " with latency " << nlatency << std::endl;
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
                std::cout << nname << " -> " << it->second << std::endl;
            }

            N -> setName(nname);
            Neurons.insert(std::make_pair(nname, N));
        }

        for (const auto& e: Ensembles) {
            std::cout << e.first << " -";
            for (const auto& en: e.second) {
                std::cout << " " << en;
            }
            std::cout << std::endl;
        }
    } catch (std::exception &e) {
        std::cout << "Error parsing structure: " << e.what() << std::endl;
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
    //for (auto N: Neurons) delete std::get<2>(N);
}
