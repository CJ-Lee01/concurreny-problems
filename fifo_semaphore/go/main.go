// Channel-based FIFO semaphore implementation:
// A coordinator goroutine owns the permit count and waiter queue. Acquire sends
// a private channel, and the coordinator replies to waiters in FIFO order.
package main

import (
	"fmt"
	"sync"
)

type FifoSemaphore struct {
	acquire chan chan struct{}
	release chan struct{}
}

func NewFifoSemaphore(permits int) *FifoSemaphore {
	s := &FifoSemaphore{
		acquire: make(chan chan struct{}),
		release: make(chan struct{}),
	}
	go func() {
		queue := make([]chan struct{}, 0)
		for {
			var next chan struct{}
			if permits > 0 && len(queue) > 0 {
				next = queue[0]
			}
			select {
			case waiter := <-s.acquire:
				queue = append(queue, waiter)
			case <-s.release:
				permits++
			case next <- struct{}{}:
				queue = queue[1:]
				permits--
			}
		}
	}()
	return s
}

func (s *FifoSemaphore) Acquire() {
	ready := make(chan struct{})
	s.acquire <- ready
	<-ready
}

func (s *FifoSemaphore) Release() {
	s.release <- struct{}{}
}

func main() {
	sem := NewFifoSemaphore(1)
	var wg sync.WaitGroup
	for id := 0; id < 4; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			sem.Acquire()
			fmt.Println("fifo worker", id)
			sem.Release()
		}(id)
	}
	wg.Wait()
}
