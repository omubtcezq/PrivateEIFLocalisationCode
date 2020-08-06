import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse,Circle
import numpy as np


# Defaults when this file is run
TRACK_FILEPATH_DEFAULT = "../input/track1.txt"
OUTPUT_FILEPATH_DEFAULT = "../output/encoding_nav001.txt"
SENSOR_LOCATIONS_DEFAULT = [np.array([5.0, 5.0]), # Normal
                            np.array([40.0, 5.0]), 
                            np.array([5.0, 40.0]), 
                            np.array([40.0, 40.0]),
                            np.array([-30.0, -30.0]), # Big
                            np.array([75.0, -30.0]), 
                            np.array([-30.0, 75.0]), 
                            np.array([75.0, 75.0]),
                            np.array([-65.0, -65.0]), # Very big 
                            np.array([110.0, -65.0]), 
                            np.array([-65.0, 110.0]), 
                            np.array([110.0, 110.0]),
                            np.array([22.0, 22.0]), # Small
                            np.array([23.0, 22.0]), 
                            np.array([22.0, 23.0]), 
                            np.array([23.0, 23.0])]
NUM_SENSORS_DEFAULT = 16


def plot_sim(track_filepath, output_filepath, sensor_locations=None, num_sensors=None):
    fig = plt.figure()
    fig.set_size_inches(w=8, h=8)
    ax = fig.add_subplot(111)

    gts = []
    states = []
    covariances = []

    with open("../input/track1.txt") as in_f:
        with open("../output/encoding_nav001.txt") as out_f:
            t = int(in_f.readline())
            dim = int(in_f.readline())
            init_state = np.array([float(x) for x in in_f.readline().split()])
            init_cov = np.array([[float(x) for x in in_f.readline().split()] for _ in range(dim)])

            for _ in range(t):
                gts.append(np.array([float(x) for x in in_f.readline().split()]))
                states.append(np.array([float(x) for x in out_f.readline().split()]))
                covariances.append(np.array([[float(x) for x in out_f.readline().split()] for _ in range(dim)]))


    # Plot initial estimate
    ax.scatter(init_state[0], init_state[2], marker='x', color='red')
    ax.add_artist(get_cov_ellipse(np.array([[init_cov[0][0], init_cov[0][2]],[init_cov[2][0], init_cov[2][2]]]), 
                                        np.array([init_state[0],init_state[2]]), 
                                        2, fill=False, linestyle='-', edgecolor='orange', zorder=1))


    # Plot ground truth
    ax.plot([x[0] for x in gts], [x[2] for x in gts], marker='.', color='gray')
    ax.plot([x[0] for x in states], [x[2] for x in states], marker='x', color='green')

    # plot filter estimates
    for state, cov in zip(states, covariances):
        ax.add_artist(get_cov_ellipse(np.array([[cov[0][0], cov[0][2]],[cov[2][0], cov[2][2]]]), 
                                        np.array([state[0],state[2]]), 
                                        2, fill=False, linestyle='-', edgecolor='royalblue', zorder=1))


    if sensor_locations != None:
        if num_sensors == None:
            num_sensors = len(sensor_locations)

        for s in range(num_sensors):
            ax.scatter(sensor_locations[s][0], sensor_locations[s][1], marker='o', color='red')

    # display the picture
    plt.show()



# Plotting helpers, from https://scipython.com/book/chapter-7-matplotlib/examples/bmi-data-with-confidence-ellipses/
def get_cov_ellipse(cov, centre, nstd, **kwargs):
    """
    Return a matplotlib Ellipse patch representing the covariance matrix
    cov centred at centre and scaled by the factor nstd.

    """

    # Find and sort eigenvalues and eigenvectors into descending order
    eigvals, eigvecs = np.linalg.eigh(cov)
    order = eigvals.argsort()[::-1]
    eigvals, eigvecs = eigvals[order], eigvecs[:, order]

    # The anti-clockwise angle to rotate our ellipse by 
    vx, vy = eigvecs[:,0][0], eigvecs[:,0][1]
    theta = np.arctan2(vy, vx)

    # Width and height of ellipse to draw
    width, height = 2 * nstd * np.sqrt(eigvals) # eigvals positive because covariance is positive semi definite
    return Ellipse(xy=centre, width=width, height=height,
                   angle=np.degrees(theta), **kwargs)



if __name__ == '__main__':
    plot_sim(TRACK_FILEPATH_DEFAULT, OUTPUT_FILEPATH_DEFAULT, SENSOR_LOCATIONS_DEFAULT, NUM_SENSORS_DEFAULT)