package utils

import "sync"

type (
	Batcher interface {
		Schedule(f func())
		Close()
		Wait()
	}

	internalScheduler struct {
		batchChannel chan (func())
		wg           sync.WaitGroup
	}
)

func NewJobScheduler(maxSimultaniousJobs uint) Batcher {
	scheduler := internalScheduler{}
	scheduler.batchChannel = make(chan func())

	for i := uint(0); i < maxSimultaniousJobs; i++ {
		go scheduler.execute()
	}
	return &scheduler
}

func (i *internalScheduler) Schedule(f func()) {
	i.wg.Add(1)
	i.batchChannel <- f
}

func (i *internalScheduler) Wait() {
	i.wg.Wait()
}

func (i *internalScheduler) Close() {
	i.Wait()
	close(i.batchChannel)
}

func (i *internalScheduler) execute() {
	for f := range i.batchChannel {
		f()
		i.wg.Done()
	}
}
