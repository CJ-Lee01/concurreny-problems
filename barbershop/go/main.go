// Channel-based barbershop implementation:
// A buffered channel is the waiting room. Customers use a nonblocking send to
// sit if a chair is free, otherwise they leave; the barber ranges over arrivals.
package main

import (
	"fmt"
	"sync"
)

func main() {
	waitingRoom := make(chan int, 2)
	done := make(chan struct{})
	var wg sync.WaitGroup

	go func() {
		served := 0
		for customer := range waitingRoom {
			served++
			fmt.Println("barber served customer", customer, "as", served)
		}
		close(done)
	}()

	for id := 1; id <= 5; id++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			select {
			case waitingRoom <- id:
				fmt.Println("customer", id, "waited")
			default:
				fmt.Println("customer", id, "left")
			}
		}(id)
	}

	wg.Wait()
	close(waitingRoom)
	<-done
}
