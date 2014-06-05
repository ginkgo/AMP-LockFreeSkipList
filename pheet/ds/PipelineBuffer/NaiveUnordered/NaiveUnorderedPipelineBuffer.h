/*
 * NaiveUnorderedPipelineBuffer.h
 *
 *  Created on: Mar 14, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef NAIVEUNORDEREDPIPELINEBUFFER_H_
#define NAIVEUNORDEREDPIPELINEBUFFER_H_

namespace pheet {

template <class Payload, class Granularity>
struct NaiveUnorderedPipelineBufferElement {
	typedef NaiveUnorderedPipelineBufferElement<Payload, class Granularity> Self;

	NaiveUnorderedPipelineBufferElement()
	: tail(nullptr), place(nullptr) {
		for(size_t i = 0; i < Granularity - 1; ++i) {
			data[i] = nullptr;
		}
	}

	Payload* data[Granularity - 1];
	Self* next;
	size_t offset;
};

template <class Pheet, class NextStage, class Payload, size_t Granularity>
class NaiveUnorderedPipelineBuffer {
public:
	typedef NaiveUnorderedPipelineBufferElement<Payload> Element;
	typedef NaiveUnorderedPipelineBuffer<Pheet, Payload, Granularity> Self;

	NaiveUnorderedPipelineBuffer();
	~NaiveUnorderedPipelineBuffer();

	void put(size_t element_id, Payload& data);
	template <class Payloads...>
	void put(size_t element_id, Payload& data, Payloads& ... rest);

private:
	size_t index;

	Element* buffer;
	Element* tail;
};

template <class Pheet, class NextStage, class Payload, size_t Granularity>
NaiveUnorderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::NaiveUnorderedPipelineBuffer()
: index(0), buffer(new Element), tail(nullptr) {
	assert(Granularity > 0);
	buffer->offset = 0;
}

template <class Pheet, class NextStage, class Payload, size_t Granularity>
NaiveUnorderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::~NaiveUnorderedPipelineBuffer() {
//	pheet_assert(buffer == nullptr);
	Element* tmp = buffer;
	while(tmp != nullptr) {
		Element* tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
}

template <class Pheet, class NextStage, class Payload, size_t Granularity>
void NaiveUnorderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::put(size_t element_id, Payload& data) {
	if(Granularity == 1) {
		Pheet::template
			spawn<NextStage>(data);
	}
	Payload* pl = new Payload(data);

	// Random starting point to better handle contention
	size_t off = SIZET_FETCH_AND_ADD(&index, 1);
	size_t i = Pheet::template rand_int(Granularity - 2);

	Element* buf = buffer;
	while((buf->offset + Granularity) <= off) {
		buf = buf->next;
	}

	bool done = false;
	size_t i = 0;
	do {
		if(buf->data[i] == nullptr) {
			if(PTR_CAS(&(buf->data[i]), nullptr, pl)) {
				done = true;
				break;
			}
		}

		i = (i + 1) % (Granularity - 1);
	} while(i != start);

	if(!done) {
		if(buffer->next != nullptr) {
			if(buffer == buf) {
				// We need
				do {
					if(buf->data[i] == nullptr) {
						if(PTR_CAS(&(buf->data[i]), nullptr, pl)) {
							done = true;
							break;
						}
					}

					i = (i + 1) % (Granularity - 1);
				} while(i != start);
			}
		}
		else {
			if(buffer == buf) {
				Element* next = new Element();

				if(PTR_CAS(&buffer, buf, next)) {
					// success
					process_buffer_element(data, buf);
				}
				else {
					// some other thread was faster
					delete next;
					// retry
					put(element_id, data);
				}
			}
			else {
				// retry
				put(element_id, data);
			}
		}


	}


}


template <class Pheet, class NextStage, class Payload, size_t Granularity>
template <class Payloads...>
void NaiveUnorderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::put(size_t element_id, Payload& data, Payloads& ... rest) {
	put(element_id, data);
	put(element_id + 1, std::forward<Payloads&>(rest) ...);
}



}

#endif /* NAIVEUNORDEREDPIPELINEBUFFER_H_ */
