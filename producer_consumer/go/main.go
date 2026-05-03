// Channel-based producer-consumer implementation:
// A buffered channel is the bounded FIFO. Sending blocks when it is full, and
// ranging over the channel consumes values until the producer closes it.
package main

import (
	"fmt"
	"sync"
)

func main() {
	buffer := make(chan int, 2)
	var wg sync.WaitGroup
	wg.Add(2)

	go func() {
		defer wg.Done()
		defer close(buffer)
		for value := 1; value <= 5; value++ {
			buffer <- value
		}
	}()

	go func() {
		defer wg.Done()
		for value := range buffer {
			fmt.Println("consumed", value)
		}
	}()

	wg.Wait()
}
