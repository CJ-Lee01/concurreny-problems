// Channel-based H2O implementation:
// Buffered channels limit hydrogen and oxygen admission. A channel barrier waits
// for exactly three admitted atoms before the tokens are released.
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
	hydrogen := make(chan struct{}, 2)
	oxygen := make(chan struct{}, 1)
	barrier := NewBarrier(3)
	var wg sync.WaitGroup

	h := func() {
		defer wg.Done()
		hydrogen <- struct{}{}
		fmt.Println("H ready")
		barrier.Wait()
		<-hydrogen
	}
	o := func() {
		defer wg.Done()
		oxygen <- struct{}{}
		fmt.Println("O ready")
		barrier.Wait()
		<-oxygen
	}

	for i := 0; i < 4; i++ {
		wg.Add(1)
		go h()
	}
	for i := 0; i < 2; i++ {
		wg.Add(1)
		go o()
	}
	wg.Wait()
}
