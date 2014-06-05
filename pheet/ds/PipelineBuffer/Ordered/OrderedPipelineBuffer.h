/*
 * OrderedPipelineBuffer.h
 *
 *  Created on: Mar 19, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ORDEREDPIPELINEBUFFER_H_
#define ORDEREDPIPELINEBUFFER_H_

#include "OrderedPipelineBufferProcessTask.h"

namespace pheet {

template <class Payload, class Granularity>
struct OrderedPipelineBufferElement {
	typedef OrderedPipelineBufferElement<Payload, Granularity> Self;

	OrderedPipelineBufferElement()
	: next(nullptr) {

	}

	Payload data[Granularity];
	Self* next;
	size_t offset;
	size_t block_counter;
};

template <class Pheet, class NextStage, class Payload, size_t Granularity>
class OrderedPipelineBuffer {
	static_assert(Granularity > 0, "Only granularities > 0 are allowed for pipeline buffers");
public:
	typedef OrderedPipelineBufferElement<Payload, Granularity> Element;

	OrderedPipelineBuffer();
	~OrderedPipelineBuffer();

	void put(size_t element_id, Payload& data);
	template <class Payloads...>
	void put(size_t element_id, Payload& data, Payloads& ... rest);

	bool prepare_next();
	void execute_next();
private:
	Element* fill_head;
	Element* process_head;
};


template <class Pheet, class NextStage, class Payload, size_t Granularity>
OrderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::OrderedPipelineBuffer()
: head(new Element()) {
	buffer->offset = 0;
}

template <class Pheet, class NextStage, class Payload, size_t Granularity>
OrderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::~OrderedPipelineBuffer() {
//	pheet_assert(buffer == nullptr);
	Element* tmp = buffer;
	while(tmp != nullptr) {
		Element* tmp2 = tmp->next;
		delete tmp;
		tmp = tmp2;
	}
}

template <class Pheet, class NextStage, class Payload, size_t Granularity>
bool OrderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::prepare_next() {
	assert(process_head != nullptr);

	Element* dummy = (Element*)this;

	while(fill_head->block_counter == Granularity) {
		if(fill_head->next == nullptr) {
			create_next(fill_head);
		}

		fill_head = fill_head->next;
	}
	while(fill_head->next != nullptr) {

	}
}

template <class Pheet, class NextStage, class Payload, size_t Granularity>
void OrderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::put(size_t element_id, Payload& data) {
	Element* buf = buffer;

	while(buf != nullptr && buf->block_counter == Granularity) {
		// block is finished
		// Make sure we are the only ones manipulating the block (and make sure
	}

	while(!(buf->offset <= element_id && (buf->offset + Granularity) > element_id)) {
		if(buf->block_counter)
		buf = buf->next;
	}


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
void OrderedPipelineBuffer<Pheet, NextStage, Payload, Granularity>::put(size_t element_id, Payload& data, Payloads& ... rest) {
	put(element_id, data);
	put(element_id + 1, std::forward<Payloads&>(rest) ...);
}


}

#endif /* ORDEREDPIPELINEBUFFER_H_ */
