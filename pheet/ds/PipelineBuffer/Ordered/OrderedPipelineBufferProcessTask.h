/*
 * OrderedPipelineBufferProcessTask.h
 *
 *  Created on: Mar 19, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef ORDEREDPIPELINEBUFFERPROCESSTASK_H_
#define ORDEREDPIPELINEBUFFERPROCESSTASK_H_

namespace pheet {

template <class Pheet, class Buffer>
class OrderedPipelineBufferProcessTask : public Pheet::Task {
public:
	OrderedPipelineBufferProcessTask(Buffer& buffer);
	virtual ~OrderedPipelineBufferProcessTask();

	virtual void operator()();

private:
	Buffer& buffer;
};

template <class Pheet, class Buffer>
OrderedPipelineBufferProcessTask<Pheet, Buffer>::OrderedPipelineBufferProcessTask(Buffer& buffer)
:buffer(buffer) {

}

template <class Pheet, class Buffer>
OrderedPipelineBufferProcessTask<Pheet, Buffer>::~OrderedPipelineBufferProcessTask() {

}

template <class Pheet, class Buffer>
void OrderedPipelineBufferProcessTask<Pheet, Buffer>::operator()() {
	while(buffer.prepare_next()) {
		buffer.execute_next();
	}
}

}

#endif /* ORDEREDPIPELINEBUFFERPROCESSTASK_H_ */
