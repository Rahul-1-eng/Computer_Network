#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    double arrivalTime;
    double startServiceTime;
    double departureTime;
} Packet;

typedef struct {
    Packet *buffer;
    int head;
    int tail;
    int count;
    int capacity;
} RouterQueue;

void initQueue(RouterQueue *q, int cap) {
    q->buffer = (Packet *)malloc(cap * sizeof(Packet));
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->capacity = cap;
}

int enqueue(RouterQueue *q, Packet p) {
    if (q->count == q->capacity) return 0;
    q->buffer[q->tail] = p;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    return 1;
}

Packet dequeue(RouterQueue *q) {
    Packet p = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return p;
}

double generateExponential(double lambda) {
    double u = (double)rand() / RAND_MAX;
    while (u == 0.0 || u == 1.0) u = (double)rand() / RAND_MAX;
    return -log(1.0 - u) / lambda;
}

int main() {
    int lBytes = 1500;
    int lBits = lBytes * 8;
    double rBps = 1000000.0;
    double propS2r = 0.002;
    double propR2d = 0.002;
    double procDelay = 0.0001;
    int queueCapacity = 50;
    int numPackets = 100000;
    
    srand(42);

    double rhoValues[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 1.0, 1.1, 1.2};
    int numExperiments = 13;

    FILE *csvFile = fopen("simulationResults.csv", "w");
    fprintf(csvFile, "Traffic Intensity (rho),Generation Rate (lambda),Generated,Delivered,Dropped,Drop Probability,Avg Queueing Delay,Avg End-to-End Delay,Max Queue Occupancy\n");

    for (int i = 0; i < numExperiments; i++) {
        double rho = rhoValues[i];
        double lambda = (rho * rBps) / lBits;

        RouterQueue q;
        initQueue(&q, queueCapacity);

        int generated = 0, delivered = 0, dropped = 0, maxQ = 0;
        double totalQDelay = 0.0, totalE2eDelay = 0.0;
        
        double currentTime = 0.0;
        double lastDepartureTime = 0.0;
        double transDelay = (double)lBits / rBps;

        for (int p = 0; p < numPackets; p++) {
            currentTime += generateExponential(lambda);
            generated++;

            Packet newPkt;
            newPkt.arrivalTime = currentTime;

            while (q.count > 0 && q.buffer[q.head].departureTime <= currentTime) {
                dequeue(&q);
            }

            if (q.count == q.capacity) {
                dropped++;
            } else {
                if (q.count == 0 && lastDepartureTime <= currentTime) {
                    newPkt.startServiceTime = currentTime;
                } else {
                    newPkt.startServiceTime = lastDepartureTime;
                }
                
                newPkt.departureTime = newPkt.startServiceTime + procDelay + transDelay;
                lastDepartureTime = newPkt.departureTime;

                enqueue(&q, newPkt);
                if (q.count > maxQ) maxQ = q.count;
                delivered++;

                double qDelay = newPkt.startServiceTime - newPkt.arrivalTime;
                double e2e = qDelay + procDelay + transDelay + propS2r + propR2d;
                
                totalQDelay += qDelay;
                totalE2eDelay += e2e;
            }
        }

        double dropProb = (double)dropped / generated;
        double avgQDelay = delivered > 0 ? (totalQDelay / delivered) : 0;
        double avgE2eDelay = delivered > 0 ? (totalE2eDelay / delivered) : 0;

        fprintf(csvFile, "%.2f,%.2f,%d,%d,%d,%.6f,%.6f,%.6f,%d\n", 
                rho, lambda, generated, delivered, dropped, dropProb, avgQDelay, avgE2eDelay, maxQ);

        free(q.buffer);
    }

    fclose(csvFile);
    return 0;
}