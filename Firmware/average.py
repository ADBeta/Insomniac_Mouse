# Simple Script to calculate average of all numbers (one per line) in a file
# to determine the performance and bias of RNG methods
# - Logs Min and Max value
# Built for Insomniac
# ADBeta    11 Jan 2025

def calculate_avg(file):
    min = 0
    max = 0

    total = 0
    count = 0

    for line in file:
        try:
            # Convert each line to a float
            value = float(line.strip())

            # Accumulate to the total
            total += value
            count += 1

            # Replace min or max if needed
            if value > max:
                max = value
            if value < min:
                min = value

        # Skip any lines that do not include numbers
        except ValueError:
            continue


    # Calculate the Average if any values were read
    if count > 0:
        average = total / count
        print(f"Average: {average:.4f}    Min: {min}    Max: {max}")
    else:
        print("No valid numbers found in the file.")


# Open the log file (replace path if needed)
log_file = open("./log.csv", "r")

# Calculate and print the min, max and average
calculate_avg(log_file)
