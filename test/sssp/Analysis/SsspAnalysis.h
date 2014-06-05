/*
 * SsspAnalysis.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SSSPANALYSIS_H_
#define SSSPANALYSIS_H_

#ifdef SSSP_SIM

#include "SsspAnalysisPerformanceCounters.h"

#include <vector>
#include <math.h>
#include <iomanip>

//#include <pheet/ds/PriorityQueue/Merge/MergeHeap.h>

namespace pheet {

struct SsspAnalysisNode {
	size_t node_id;
	size_t distance;
	size_t distance_prev;
	size_t added;
	bool processed;
	bool active;
};

struct SsspAnalysisNodeLess {
	bool operator()(SsspAnalysisNode const& n1, SsspAnalysisNode const& n2) {
		return n1.distance < n2.distance;
	}
};

template <class Pheet>
class SsspAnalysis : public Pheet::Task {
public:
	typedef SsspAnalysisPerformanceCounters<Pheet> PerformanceCounters;

	SsspAnalysis(SsspGraphVertex* graph, size_t size, PerformanceCounters& pc)
	:graph(graph), size(size), pc(pc) {}
	virtual ~SsspAnalysis() {}

	virtual void operator()() {
		SsspAnalysisNodeLess less;

		// k tasks may not be visible to others
		size_t const k = sssp_sim_k;
		// Number of items processed in each phase (= #threads in model)
		size_t const block_size = sssp_sim_p;
		size_t phase = 0;

		SsspAnalysisNode n;
		n.distance = 0;
		n.node_id = 0;
		n.added = 0;
		n.processed = false;

		size_t* settled = new size_t[size];
		settled[0] = 0;

		std::vector<size_t> h_hist;
		h_hist.push_back(0);

		std::vector<double> active_p_hist;
		active_p_hist.push_back(0);

#ifdef SSSP_SIM_VERIFY_THEORY
		std::vector<double> Wt_hist;
		Wt_hist.push_back(0);

		size_t const pascal_max = 2048;
		std::vector<double*> pascal;
		{
			// First row of pascal
			double* data = new double[1];
			data[0] = 1.0;
			pascal.push_back(data);
		}
		for(size_t i = 1; i < pascal_max; ++i) {
			double* data = new double[i+1];
			for(size_t j = 0; j <= i; ++j) {
				double first, second;
				if(j == 0) {
					first = 0.0;
				}
				else {
					first = pascal[i-1][j-1];
				}
				if(j == i) {
					second = 0.0;
				}
				else {
					second = pascal[i-1][j];
				}
				data[j] = first + second;
			}

			pascal.push_back(data);
		}
#endif

		graph[0].distance = 0;

		std::vector<SsspAnalysisNode> v;
		v.push_back(n);
		size_t offset = 0;

		while(v.size() > offset) {
			n = v[offset];
			if(n.processed) {
				++offset;
				// Node was already processed
				continue;
			}
			size_t node = n.node_id;

			size_t d = graph[node].distance;
			if(d != n.distance) {
				++offset;
				pc.num_dead_tasks.incr();
				// Distance has already been improved in the meantime
				// No need to check again
				continue;
			}
			++phase;

			size_t base = v[offset].distance;
#ifndef SSSP_SIM_STRUCTURED
			std::cout << "Phase " << phase << ":" << std::endl;
			std::cout << "Nodes in list (* ... nodes not visible to all threads) " << std::endl;

			size_t sum = 0;
			size_t processed_sum = 0;
			size_t samples = 0;
			size_t processed_samples = 0;
			// Print list of nodes
			for(size_t i = offset; i < v.size(); ++i) {
				if(graph[v[i].node_id].distance == v[i].distance) {
					if(!v[i].processed) {
						++samples;
						sum += v[i].distance - base;
						if(samples < 50 || i >= v.size() - 50) {
							std::cout << v[i].distance/* - base*/;
							if(v[i].added + k >= v.size()) {
								std::cout << "*";
							}
							std::cout << ",";
						}
						else if(samples == 100) {
							std::cout << "..., ";
						}
					}
					else {
						++processed_samples;
						processed_sum += v[i].distance - base;
					}
				}
				else {
					pheet_assert(i != offset);
				}
			}
			if(samples > 0) {
				std::cout << std::endl << "Avg. Weight: " << (sum / (samples)) << " for " << samples << " nodes" << std::endl;
			}
			else {
				std::cout << std::endl;
			}
			if(processed_samples > 0) {
				std::cout << "Avg. Weight (incl processed > dist): " << ((sum + processed_sum) / (processed_samples + samples)) << " for " << (processed_samples + samples) << " nodes" << std::endl;
			}
#endif

#ifdef SSSP_SIM_DEP_CHECK
			{
				std::vector<size_t> active_nodes;
				for(size_t i = offset; i < v.size(); ++i) {
					n = v[i];
					size_t node = n.node_id;
					size_t d = graph[node].distance;
					if(d == n.distance && !n.processed) {
						active_nodes.push_back(n.node_id);
					}
				}

				size_t tested = 0;
				size_t found = 0;
				for(size_t i = 0; i < active_nodes.size() - 1; ++i) {
					for(size_t j = i + 1; j < active_nodes.size(); ++j) {
						++tested;
						auto first = graph[active_nodes[i]].edges;
						auto last = graph[active_nodes[i]].edges + graph[active_nodes[i]].num_edges - 1;
						while(first != last) {
							auto middle = first + ((last - first) >> 1);
							if(middle->target == active_nodes[j]) {
								first = last = middle;
							}
							else if(middle->target < active_nodes[j]) {
								first = middle + 1;
							}
							else {
								last = middle;
							}
						}
						if(first->target == active_nodes[j]) {
							++found;
						}
					}
				}

				active_p_hist.push_back((double)found/(double)tested);
#ifndef SSSP_SIM_STRUCTURED
				std::cout << "Connections in active set: " << found << " out of " << tested << " possible connections. Ratio: " << (double)found/(double)tested << std::endl;
#else
				active_p_hist.push_back((double)found/(double)tested);
#endif
			}
#endif

#ifdef SSSP_SIM_VERIFY_THEORY
			std::vector<size_t> processed_ref;
			std::vector<size_t> base_d;
#endif
			// Mark nodes that can be processed in this phase as active (distance values
			// might change during update, so we can't rely on them)
			for(size_t i = offset; i < v.size(); ++i) {
				if(graph[v[i].node_id].distance == v[i].distance &&
						!v[i].processed) {
					v[i].active = true;
				}
				else {
					v[i].active = false;
				}
			}

			// Go through list and only process nodes visible to all threads and the first node (which is always processed)
			size_t orig_size = v.size();
			size_t sum_new = 0;
			size_t sum_upd = 0;
			size_t max_h = 0;
			size_t j, j2;
			for(j = 0, j2 = 0; j < block_size && offset + j2 < orig_size; ++j2) {
				n = v[offset + j2];
				size_t node = n.node_id;
				size_t d = n.distance;
				size_t a = n.added;
#ifdef SSSP_SIM_VERIFY_THEORY
				if(n.active) {
					base_d.push_back(d);
				}
#endif
				pheet_assert(k > 0 || a + k < orig_size);
				// Node is visible to all threads or the first node
				if(j == 0 || a + k < orig_size) {
					// Node has not been processed, and no better distance value has been found
					if(n.active) {
						pc.num_actual_tasks.incr();

#ifdef SSSP_SIM_VERIFY_THEORY
						processed_ref.push_back(base_d.size());
#endif

						size_t h = d - base;
						if(h > max_h) {
							max_h = h;
						}

						// relax node
						for(size_t i = 0; i < graph[node].num_edges; ++i) {
							size_t new_d = d + graph[node].edges[i].weight;
							size_t target = graph[node].edges[i].target;
							size_t old_d = graph[target].distance;
							if(old_d > new_d) {
								if(old_d == std::numeric_limits<size_t>::max()) {
									++sum_new;
								}
								else {
									++sum_upd;
								}
								graph[target].distance = new_d;
								n.distance = new_d;
								n.node_id = target;
								n.processed = false;
								v.push_back(n);
							}
						}
						v[offset + j2].processed = true;
						v[offset + j2].active = false;
						// Store in which phase node has been settled (will be overwritten if same node is settled again)
						settled[node] = phase;
						++j;
					}
				}
			}

			size_t max_h_rnd = max_h;
			// If not enough nodes visible to all threads are available process some random nodes
			if(orig_size - offset > 1 && j < block_size) {
				// Shuffle nodes for randomness
				std::random_shuffle(v.begin() + offset + 1, v.begin() + orig_size);

				for(j2 = 1;j < block_size && offset + j2 < orig_size; ++j2) {
					n = v[offset + j2];
					size_t node = n.node_id;
					size_t d = graph[node].distance;
					size_t a = n.added;

					// Node not visible to all threads
					if(a + k >= orig_size) {
						// Node has not been processed, and no better distance value has been found
						if(n.active) {
							pc.num_actual_tasks.incr();

#ifdef SSSP_SIM_VERIFY_THEORY
							for(size_t bdi = 0; bdi < base_d.size(); ++bdi) {
								if(base_d[bdi] == d) {
									processed_ref.push_back(bdi + 1);
									break;
								}
								pheet_assert(bdi != base_d.size());
							}
#endif

							size_t h = d - base;
							if(h > max_h_rnd) {
								max_h_rnd = h;
							}

							// relax node
							for(size_t i = 0; i < graph[node].num_edges; ++i) {
								size_t new_d = d + graph[node].edges[i].weight;
								size_t target = graph[node].edges[i].target;
								size_t old_d = graph[target].distance;
								if(old_d > new_d) {
									if(old_d == std::numeric_limits<size_t>::max()) {
										++sum_new;
									}
									else {
										++sum_upd;
									}
									graph[target].distance = new_d;
									n.distance = new_d;
									n.node_id = target;
									n.processed = false;
									v.push_back(n);
								}
							}
							v[offset + j2].processed = true;
							settled[node] = phase;
							++j;
						}
					}
				}
			}

#ifdef SSSP_SIM_VERIFY_THEORY
			double Wt = 0;
			std::sort(processed_ref.begin(), processed_ref.end());
			for(size_t tjr = 1; tjr <= processed_ref.size(); ++tjr) {
				size_t tj = processed_ref[tjr - 1];
				double qt = 1.0;
				for(size_t ti = 1; ti < tj; ++ti) {
					double h = ((double)(base_d[tj-1] - base_d[ti-1]))/sssp_sim_theory_div;
					double ph = sssp_sim_theory_p * h;
					// Use approximation (due to floating point limitations)?
					bool approx = false;
					for(size_t L = 1; L < size; ++L) {
						pheet_assert(base_d[tj-1] >= base_d[ti-1]);
						if(!approx) {
							double iptL = 0;
							double tmp = ph;
							for(size_t L_fac = L; L_fac > 1; --L_fac) {
								tmp *= ph/ ((double) L_fac);
							}
							if(tmp == 0.0) {
								break;
							}

							// Manually exponentiate (1 - tmp) to omit rounding errors (first iteration should hopefully be enough)
							if(L > 1) {
								// Precalculate (1-tmp)^x for all x that are powers of 2 until pascal_max
								size_t ptLexp_offset = 2;
								size_t exponent = size - 2;
								while(exponent < pascal_max && ptLexp_offset < L) {
									exponent *= (size - ptLexp_offset - 1);
									++ptLexp_offset;
								}
								std::vector<double> calculated;
								calculated.push_back(1-tmp);
								bool pointless = false;
								for(size_t ei = 1; ((size_t)1 << ei) <= pascal_max && !pointless; ++ei) {
									double val = 0.0;
									double pow_tmp = 1.0;
									size_t cur_exp = 1 << ei;
									for(size_t ej = 0; ej < cur_exp; ++ej) {
										val += pascal[cur_exp - 1][ej] * pow_tmp;
										pow_tmp *= -tmp;
										if(pow_tmp == 0.0) {
											pointless = true;
											break;
										}
									}
									// Sanity check for rounding errors
									if(val > 1.0) {
										val = 1.0;
									}
									calculated.push_back(val);
								}
								// Prepare more exponents until we reach what we need
								while(exponent > ((size_t)1 << calculated.size())) {
									calculated.push_back(calculated.back()*calculated.back());
								}

								// Now greedily exponentiate with the calculated values
								size_t greed = calculated.size() - 1;
								iptL = 1.0;
								while(exponent > 0) {
									size_t greed_exp = 1 << greed;
									while(greed_exp <= exponent) {
										iptL *= calculated[greed];
										exponent -= greed_exp;
									}
									--greed;
								}


						//		iptL = 1 - tmp;
						/*		if(iptL == 1.0) {
									std::cout << "aha";
								}*/
								// Calculate more exponents
								for(size_t ptLexp = ptLexp_offset; ptLexp < L; ++ptLexp) {
									iptL = pow(iptL, (double)(size - ptLexp - 1));
								}
						//		std::cout << "tj " << tj << " ti " << ti << " L " << L << " h " << h << " tmp " << tmp << " iptL " << iptL << std::endl;
								if(iptL == 1.0) {
									// Switch to approximation
									approx = true;
								}
						/*		if(iptL == 1.0) {
									std::cout << "aha2";
								}*/
								qt *= iptL;
							}
							else {
								iptL = 1 - tmp;
								qt *= iptL;
						//		std::cout << "tj " << tj << " ti " << ti << " L " << L << " h " << h << " tmp " << tmp << " iptL " << iptL << std::endl;
							}
						}

						// Due to floating point problems approximate the rest (approximation is good for large L!)
						if(approx)
						{
							double apv = 1.0 / (size - 1);
							for(size_t binomial = 0; binomial < L; ++binomial) {
								apv *= (size - 1 - binomial);
								apv *= ph;
								apv /= (L - binomial);
							}
						//	std::cout << std::setprecision(48) << -apv;
							apv = exp(-apv);
						//	std::cout << "tj " << tj << " ti " << ti << " L " << L << " h " << h << " approx " << std::setprecision(48) << apv << std::endl;
							if(apv == 1.0) {
								break;
							}
							qt *= apv;
						}
					}
				}
//				std::cout << "qt " << qt << std::endl;
				Wt += 1.0 - qt;
			}
			Wt_hist.push_back(((double)processed_ref.size()) - Wt);
		//	std::cout << "Wt " << Wt << std::endl;
			processed_ref.clear();
			base_d.clear();
#endif

			// Shuffle newly added nodes so the nodes not visible to all threads are really random
			std::random_shuffle(v.begin() + orig_size, v.end());
			for(size_t i2 = orig_size; i2 < v.size(); ++i2) {
				// Store information that tells us whether node is visible to all threads
				v[i2].added = i2;
			}

			++offset;
#ifndef SSSP_SIM_STRUCTURED
			std::cout << "New nodes found: " << sum_new << std::endl << "Better distance value found for " << sum_upd << " nodes" << std::endl;
			std::cout << "Maximum h: " << max_h << ", including randomly selected nodes: " << max_h_rnd << std::endl;
			std::cout << "------------------" << std::endl;
#endif
			h_hist.push_back(max_h_rnd);

			// Sort nodes by distance value for next iteration
			std::sort(v.begin() + offset, v.end(), less);
		}

#ifdef SSSP_SIM_VERIFY_THEORY
		for(size_t i = 0; i < pascal_max; ++i) {
			delete[] pascal[i];
		}
#endif

		for(size_t i = 0; i <= phase; ++i) {
			size_t counter = 0;
			for(size_t j = 0; j < size; ++j) {
				if(settled[j] == i) {
					++counter;
				}
			}
#ifndef SSSP_SIM_STRUCTURED
			std::cout << "Nodes settled in phase " << i << ": " << counter << std::endl;
#ifdef SSSP_SIM_VERIFY_THEORY
			std::cout << "Expected settled nodes " << Wt_hist[i] << std::endl;
#endif
#else
			std::cout << "SSSP_SIM_DATA\t" << i << "\t" << counter << "\t" << h_hist[i];
#ifdef SSSP_SIM_DEP_CHECK
			std::cout << "\t" << active_p_hist[i];
#endif
#ifdef SSSP_SIM_VERIFY_THEORY
			std::cout << "\t" << Wt_hist[i];
#endif
			std::cout << std::endl;
#endif
		}
		delete[] settled;
	}

	static void set_k(size_t k) {}

	static char const name[];
private:
	SsspGraphVertex* graph;
	size_t size;
	PerformanceCounters pc;
};

template <class Pheet>
char const SsspAnalysis<Pheet>::name[] = "Sssp Analysis";

} /* namespace pheet */

#endif

#endif /* SSSPANALYSIS_H_ */
