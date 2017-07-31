
def load_perf_file(file, method=""):
    if method == "":
        method = os.path.basename(file).split('_')[0]

    with open(file, 'r') as f:
        s = f.read()

    genome = os.path.basename(file).split('_')[1]
    region = os.path.basename(file).split('_')[2].split('.perf')[0]

    lines = s.split('\n')
    usertime = float(lines[1].split(': ')[1])
    systemtime = float(lines[2].split(': ')[1])
    cpu_usage = float(lines[3].split(': ')[1].split('%')[0])
    memory = int(lines[9].split(': ')[1].split('%')[0])

    return [genome, method, region, usertime, systemtime, cpu_usage, memory]


def get_metric(df, column, regions):
    return np.array([df[df['region'] == r][column].values for r in regions])


def compare_metrics(df, methods, regions, column, save=False):
    plt.figure(figsize=(25, 6))
    dfs = [df[df['method'] == m] for m in methods]
    metrics = [get_metric(d, column, regions) for d in dfs]
    x = np.arange(len(regions))
    N = len(methods) + 1
    for i, (m, method) in enumerate(zip(metrics, methods)):
        plt.bar(x+1/N*i, m, width=1/N, label=method)
    plt.title(column, fontsize=25)
    plt.legend(fontsize=16)
    if save:
        plt.savefig("images/{0}.png".format(column), format='png')
#     plt.yscale('log')
