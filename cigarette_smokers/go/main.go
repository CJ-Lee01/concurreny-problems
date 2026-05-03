// Channel-based cigarette smokers implementation:
// The agent channel serializes table placements. Each smoker waits on its own
// channel and signals the agent after making a cigarette.
package main

import (
	"fmt"
	"sync"
)

func main() {
	agent := make(chan struct{}, 1)
	tobacco := make(chan struct{})
	paper := make(chan struct{})
	matches := make(chan struct{})
	var wg sync.WaitGroup
	agent <- struct{}{}

	smoker := func(name string, need <-chan struct{}) {
		defer wg.Done()
		for range need {
			fmt.Println(name, "smoker made a cigarette")
			agent <- struct{}{}
		}
	}

	wg.Add(3)
	go smoker("tobacco", tobacco)
	go smoker("paper", paper)
	go smoker("matches", matches)

	for round := 0; round < 6; round++ {
		<-agent
		switch round % 3 {
		case 0:
			tobacco <- struct{}{}
		case 1:
			paper <- struct{}{}
		default:
			matches <- struct{}{}
		}
	}

	<-agent
	close(tobacco)
	close(paper)
	close(matches)
	wg.Wait()
}
