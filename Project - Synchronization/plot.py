import matplotlib.pyplot as plt

def main():
    filename = 'out.txt'

    with open(filename, 'r') as f:
        data = f.read()

    seats = []
    not_received = []

    for line in data.split('\n'):
        if line.strip():
            label, value = line.split('=')
            if 'number of seats' in label:
                seats.append(int(value.strip()))
            elif '# customers who didn\'t receive a service' in label:
                not_received.append(int(value.strip()))
    # print(seats)
    # print(not_received)
    plt.bar(seats, not_received)
    plt.xlabel('Number of barbers')
    plt.ylabel('Number of customers who did not receive service')
    plt.title('Customers who did not receive service vs. number of seats for 1 barber')
    plt.show()

# def main():
#     filename = 'out.txt'

#     with open(filename, 'r') as f:
#         data = f.read()

#     seats = []
#     not_received = []

#     for line in data.split('\n'):
#         if line.strip():
#             label, value = line.split('=')
#             if 'number of seats' in label:
#                 seats.append(int(value.strip()))
#             elif '# customers who didn\'t receive a service' in label:
#                 not_received.append(int(value.strip()))
#     # print(seats)
#     # print(not_received)
#     plt.bar(seats, not_received)
#     plt.xlabel('Number of barbers')
#     plt.ylabel('Number of customers who did not receive service')
#     plt.title('Change is number of barbers vs. number of customers who did not receive service')
#     plt.show()


if __name__ == '__main__':
    main()
