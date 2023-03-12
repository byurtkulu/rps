import multiprocessing
import createGameSessionLoop
# Define the function to run script1.py
def run_script():
    createGameSessionLoop.main()

if __name__ == '__main__':
    list_of_processes = []
    # Create two processes to run the scripts in parallel
    num_of_processes = int(input("Enter number of processes: "));
    for i in range(0, num_of_processes):
        print(f"Creating process {i}")
        list_of_processes.append(multiprocessing.Process(target=run_script))

    for p in list_of_processes:
        print(f"Starting process {p}")
        p.start()

    for p in list_of_processes:
        print(f"Joining process {p}")
        p.join()

