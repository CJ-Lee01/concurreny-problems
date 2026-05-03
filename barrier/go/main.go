// Channel-based barrier implementation:
// A coordinator goroutine collects one private channel per arriving goroutine.
// Once n arrivals are queued, it closes every private channel to release them.
package main

import (
	"fmt"
	"sync"
)

type Barrier struct {
	arrive chan chan struct{}
}

func NewBarrier(n int) *Barrier {
	b := &Barrier{arrive: make(chan chan struct{})}
	go func() {
		waiters := make([]chan struct{}, 0, n)
		for ready := range b.arrive {
			waiters = append(waiters, ready)
			if len(waiters) == n {
				for _, waiter := range waiters {
					close(waiter)
				}
				waiters = make([]chan struct{}, 0, n)
			}
		}
	}()
	return b
}

func (b *Barrier) Wait() {
	ready := make(chan struct{})
	b.arrive <- ready
	<-ready
}

func main() {
	barrier := NewBarrier(3)
	var wg sync.WaitGroup
	for id := 0; id < 3; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			fmt.Println("worker", id, "before barrier")
			barrier.Wait()
			fmt.Println("worker", id, "after barrier")
		}(id)
	}
	wg.Wait()
}
