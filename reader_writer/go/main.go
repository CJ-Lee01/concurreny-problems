// Channel-based reader-writer implementation:
// A single owner goroutine holds the shared value. Readers and writers send
// requests over channels, avoiding mutexes and atomics entirely.
package main

import (
	"fmt"
	"sync"
)

type readRequest struct {
	reply chan int
}

type writeRequest struct {
	value int
	done  chan struct{}
}

func main() {
	reads := make(chan readRequest)
	writes := make(chan writeRequest)
	stop := make(chan struct{})

	go func() {
		value := 0
		for {
			select {
			case request := <-reads:
				request.reply <- value
			case request := <-writes:
				value = request.value
				close(request.done)
			case <-stop:
				return
			}
		}
	}()

	done := make(chan struct{})
	writes <- writeRequest{value: 42, done: done}
	<-done
	fmt.Println("writer stored 42")

	var wg sync.WaitGroup
	for id := 0; id < 3; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			reply := make(chan int)
			reads <- readRequest{reply: reply}
			fmt.Println("reader", id, "saw", <-reply)
		}(id)
	}
	wg.Wait()
	close(stop)
}
