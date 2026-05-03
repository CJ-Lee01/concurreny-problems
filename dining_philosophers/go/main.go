// Channel-based dining philosophers implementation:
// Each fork is a buffered channel containing one token. Philosophers receive
// the lower-numbered fork token first, preventing circular wait.
package main

import (
	"fmt"
	"sync"
)

func main() {
	forks := make([]chan struct{}, 5)
	for i := range forks {
		forks[i] = make(chan struct{}, 1)
		forks[i] <- struct{}{}
	}

	var wg sync.WaitGroup
	for id := 0; id < 5; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			left, right := id, (id+1)%5
			first, second := left, right
			if second < first {
				first, second = second, first
			}
			<-forks[first]
			<-forks[second]
			fmt.Println("philosopher", id, "ate")
			forks[second] <- struct{}{}
			forks[first] <- struct{}{}
		}(id)
	}
	wg.Wait()
}
