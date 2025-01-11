# Simple Script to calculate average of all numbers (one per line) in a file
# to determine the performance and bias of RNG methods
# Built for Insomniac
# ADBeta    11 Jan 2025

def calculate_avg(file_path):
    total = 0
    count = 0

    try:
        with open(file_path, 'r') as file:
            for line in file:
                try:
                    # Convert each line to a float and accumulate the total
                    total += float(line.strip())
                    count += 1

                # Skip any lines that do not include numbers
                except ValueError:
                    continue

        # Calculate the Average if any values were read
        if count > 0:
            average = total / count
            print(f"Average: {average:.4f}")
        else:
            print("No valid numbers found in the file.")
            
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")


# Open the log file (replace path if needed)
file_path = "./log.csv"
# Calculate and print the average
calculate_avg(file_path)
