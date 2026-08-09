# Monitor

Observability stack for [**Prismo**](../README.md) benchmarks, packaged as a single Docker Compose file. It combines live host metrics scraped by Prometheus with offline campaign metrics collected by `pcp dstat`, both rendered through pre-provisioned Grafana dashboards. The two views share the same panel layout, so a live run and an archived campaign can be compared side by side.

## Stack

| Service             | Image                              | Description                                                                     | Port    |
|---------------------|------------------------------------|---------------------------------------------------------------------------------|---------|
| **grafana**         | `grafana/grafana`                  | Dashboards, datasources, and panels, provisioned automatically at startup       | `3000`  |
| **prometheus**      | `prom/prometheus`                  | Scrapes and stores the live metrics exposed by the exporter                     | `9090`  |
| **node-exporter**   | `prom/node-exporter`               | Exposes host CPU, memory, disk, and network counters                            | `9101`  |
| **csv-server**      | `nginx:alpine`                     | Serves the campaign results directory as static files over HTTP                 | *(internal)* |
| **renderer**        | `grafana/grafana-image-renderer`   | Renders panels and dashboards as images, used for exports and reports           | *(internal)* |

> [!NOTE]
> `csv-server` and `renderer` are only reachable from inside the Compose network. They are not published to the host because nothing outside Grafana needs to talk to them.

## Prerequisites

1. Install Docker and the Compose plugin

```sh
sudo apt update
sudo apt install -y docker.io docker-compose-plugin
```

2. Install [**PCP**](https://pcp.io/) to collect the per-workload CSV files consumed by the campaign dashboard

```sh
sudo apt install -y pcp
```

3. Create the environment file and define the token shared between Grafana and the renderer

```sh
cp .env.example .env
```

```sh
GF_RENDERING_RENDERER_TOKEN=my_secret_token
```

## Usage

```sh
# Start the whole stack in the background
docker compose up -d

# Follow the logs of a single service
docker compose logs -f grafana

# Stop the stack, keeping dashboards and stored metrics
docker compose down

# Stop the stack and discard all persisted data
docker compose down -v
```

### Endpoints

| Service         | URL                                                          | Description                                    |
|-----------------|--------------------------------------------------------------|------------------------------------------------|
| Grafana         | [http://localhost:3000](http://localhost:3000)               | Dashboards, default credentials `admin/admin`  |
| Prometheus      | [http://localhost:9090](http://localhost:9090)               | Query browser and scrape target status         |
| Node Exporter   | [http://localhost:9101/metrics](http://localhost:9101/metrics) | Raw metrics, useful to confirm collection      |

> [!IMPORTANT]
> Two named volumes, `grafana-data` and `prometheus-data`, hold dashboard state and the metric database. Running `docker compose down -v` deletes both, so any manual dashboard edits are lost.

## Dashboards

Dashboards live in [**grafana/dashboards**](grafana/dashboards/) and are provisioned into the `Benchmark` folder by [**dashboards.yml**](grafana/provisioning/dashboards/dashboards.yml). They are marked as editable, but changes made in the UI are overwritten whenever the file on disk is reloaded, so edits should be exported back into the JSON.

| Dashboard                                                            | Datasource   | Description                                                            |
|----------------------------------------------------------------------|--------------|-------------------------------------------------------------------------|
| [**System Metrics**](grafana/dashboards/system-metrics.json)         | Prometheus   | Live host metrics, refreshed every 5 seconds while a benchmark runs     |
| [**Campaign Results**](grafana/dashboards/campaign-csv.json)         | Infinity     | Replays the `.dstat.csv` files produced by a finished campaign          |

---

### System Metrics

Live view backed by `node-exporter`, intended to be watched while a workload is executing. All panels are rate-based over a one minute window, so short spikes are smoothed out.

| Panel                | Description                                                       |
|----------------------|---------------------------------------------------------------------|
| `CPU Usage`          | User, system, iowait, steal, and idle time, averaged across cores   |
| `CPU Events`         | Interrupt and context switch rates                                  |
| `Memory Usage`       | Used, buffered, cached, and free memory                             |
| `Disk Throughput`    | Bytes read and written per second                                   |
| `Disk IOPS`          | Completed read and write operations per second                      |
| `Disk Latency`       | Average service time per read and per write                         |
| `Disk Queue Depth`   | Requests currently in flight                                        |
| `Disk Paging`        | Pages swapped in and out                                            |
| `Network Throughput` | Bytes received and transmitted per second                           |
| `Network Packets`    | Packets received and transmitted per second                         |

Loopback and virtual devices are filtered out of the queries, keeping the panels focused on the hardware actually under test.

```sh
# Excluded block devices
loop.* | dm-.*

# Excluded network interfaces
lo | veth.* | docker.* | br-.*
```

> [!WARNING]
> The exporter runs with `pid: host` and read-only bind mounts of `/proc`, `/sys`, and `/`, so it reports host-wide activity. Any other process running on the machine during a benchmark shows up in these panels.

---

### Campaign Results

Offline view of a campaign that has already finished. Instead of querying Prometheus, panels fetch a CSV file over HTTP through the Infinity datasource, which means results can be inspected long after the run, on a machine that never executed the benchmark.

The dashboard is parameterised by two textbox variables that together build the request URL.

| Variable    | Description                                       | Example                      |
|-------------|-----------------------------------------------------|------------------------------|
| `campaign`  | Campaign folder name inside the results directory | `campaign_20260324_214924`   |
| `workload`  | Workload name, without the `.dstat.csv` suffix    | `01_seq_write_posix`         |

```sh
http://csv-server:8080/${campaign}/${workload}.dstat.csv
```

The panels mirror a subset of [**System Metrics**](#system-metrics), reading the columns emitted by `pcp dstat`.

| Panel                | Columns                                                             |
|----------------------|-----------------------------------------------------------------------|
| `CPU Usage`          | `total usage:usr`, `total usage:sys`, `total usage:idl`, `total usage:wai`, `total usage:stl` |
| `CPU Events`         | `int`, `csw`                                                          |
| `Memory Usage`       | `used`, `free`, `buf`, `cach`                                         |
| `Disk Throughput`    | `dsk/total:read`, `dsk/total:writ`                                    |
| `Disk IOPS`          | `read`, `writ`                                                        |
| `Disk Paging`        | `in`, `out`                                                           |
| `Network Throughput` | `net/total:recv`, `net/total:send`                                    |

## Collecting campaign data

The CSV files are produced by the runners in [**pcp-dstat**](../tools/pcp-dstat/), which wrap each workload with a `pcp dstat` collector and write both the report and the metrics into a timestamped folder.

```sh
# Run prismo workloads with metric collection
./tools/pcp-dstat/prismo.sh -b ./builddir/prismo workloads/prismo/*.json
```

```
workloads/results/
└── campaign_20260324_214924/
    ├── 01_seq_write_posix.report.json
    ├── 01_seq_write_posix.dstat.csv
    └── ...
```

The `csv-server` container bind-mounts [**workloads/results**](../workloads/results/) read-only at `/data` and exposes it with `autoindex` enabled, so the directory listing is also available as JSON.

> [!IMPORTANT]
> Results must live under `workloads/results`, which is the default output directory of [**Cardoide**](../tools/cardoide/README.md) and of the `pcp-dstat` runners. Campaigns stored elsewhere are not visible to the dashboard unless the mount in [**docker-compose.yml**](docker-compose.yml) is adjusted.

## Configuration

| File                                                                              | Description                                                     |
|-----------------------------------------------------------------------------------|-------------------------------------------------------------------|
| [**docker-compose.yml**](docker-compose.yml)                                      | Service definitions, ports, volumes, and environment variables  |
| [**prometheus/prometheus.yml**](prometheus/prometheus.yml)                        | Scrape targets and intervals, currently 5 seconds               |
| [**nginx/default.conf**](nginx/default.conf)                                      | Static file server for the results directory, with CORS enabled |
| [**grafana/provisioning/datasources**](grafana/provisioning/datasources/)         | Prometheus and Infinity datasource definitions                  |
| [**grafana/provisioning/dashboards**](grafana/provisioning/dashboards/)           | Dashboard provider pointing at the mounted dashboards folder    |

> [!NOTE]
> The Infinity datasource restricts requests to `http://csv-server:8080`. Serving results from a different host requires adding it to `allowedHosts` in [**datasources/prometheus.yml**](grafana/provisioning/datasources/prometheus.yml).

> [!TIP]
> Lowering `scrape_interval` gives finer resolution for short workloads, at the cost of a larger metric database and more overhead on the machine under test.

## Contributing

The current dashboards only cover system-level metrics. The natural next step is exposing the benchmark's own numbers, IOPS, bandwidth, and latency percentiles from the [**report**](../README.md#report), alongside the host counters.

1. Add the panels to the relevant dashboard JSON in [**grafana/dashboards**](grafana/dashboards/).

2. If a new datasource is needed, declare it in [**grafana/provisioning/datasources**](grafana/provisioning/datasources/) with a stable `uid`.

3. Restart the stack with `docker compose up -d` so the provisioning is reloaded.
