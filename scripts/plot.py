#!/usr/bin/env python3
"""
plot.py - ritar grafer från sortlab-resultaten.

Läser alla CSV-filer i results/, ritar mätpunkter med felstaplar
(stddev) och en passning till den teoretiska komplexiteten. Skapar
en PNG per CSV-fil i plots/.

Skälet att vi har en separat passning är att labbinstruktionen kräver
"en passning till den algoritmiska komplexitet som ges av litteraturen"
i varje graf, så vi kan visuellt jämföra mätdata mot teori.

Användning:
    python3 scripts/plot.py
    python3 scripts/plot.py --compare           # extra jämförelse-grafer

Beroenden: matplotlib, numpy
"""

import argparse
import csv
import sys
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt


# Mappa varje (algoritm, datatyp) till förväntad komplexitet.
# Anledningen att vi gör det per kombination är att t.ex. naiv quicksort
# har olika komplexitet beroende på indata: O(N log N) på random men
# O(N^2) på sorterad/konstant.
COMPLEXITY = {
    # insertion sort
    'insertion.random':      'quadratic',
    'insertion.rising':      'linear',       # best case
    'insertion.falling':     'quadratic',    # worst case
    'insertion.constant':    'linear',       # best case (inga shifts)

    # selection sort - alltid O(N^2)
    'selection.random':      'quadratic',
    'selection.rising':      'quadratic',
    'selection.falling':     'quadratic',
    'selection.constant':    'quadratic',

    # naiv quicksort
    'quicksort.random':      'linearithmic',
    'quicksort.rising':      'quadratic',
    'quicksort.falling':     'quadratic',
    'quicksort.constant':    'quadratic',

    # quicksort med median-of-three
    'quicksort_m3.random':   'linearithmic',
    'quicksort_m3.rising':   'linearithmic',
    'quicksort_m3.falling':  'linearithmic',
    'quicksort_m3.constant': 'quadratic',    # Lomuto-svaghet kvarstår

    # std::sort (introsort)
    'std_sort.random':       'linearithmic',
    'std_sort.rising':       'linearithmic',
    'std_sort.falling':      'linearithmic',
    'std_sort.constant':     'linearithmic',
}

PRETTY_LABEL = {
    'linear':        'O(N)',
    'linearithmic':  'O(N log N)',
    'quadratic':     'O(N²)',
}


def model_curve(complexity, n_arr, k):
    """Returnerar k * f(N) för den teoretiska komplexiteten."""
    n = np.asarray(n_arr, dtype=float)
    if complexity == 'linear':
        return k * n
    if complexity == 'linearithmic':
        return k * n * np.log2(n)
    if complexity == 'quadratic':
        return k * n * n
    raise ValueError(f'Okänd komplexitet: {complexity}')


def fit_constant(n_arr, t_arr, complexity):
    """
    Hittar konstanten k så att modellen passar bäst.

    Vi tar medelvärdet av t/f(N) över alla mätpunkter. Detta ger en
    rimlig "least squares"-passning eftersom vi viktar alla mätpunkter
    lika i log-rummet.
    """
    base = model_curve(complexity, n_arr, 1.0)
    # Skydda mot nollor (t.ex. om en mätning blev exakt 0 ms)
    mask = (base > 0) & (t_arr > 0)
    if not np.any(mask):
        return 0.0
    return float(np.mean(t_arr[mask] / base[mask]))


def read_csv(path):
    """Läser en CSV-fil och returnerar (method_name, N, T, Stdev) som arrays."""
    method = path.stem
    n_arr, t_arr, e_arr = [], [], []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith('#'):
                # Vi extraherar method-namnet från kommentaren om det finns
                if 'method=' in line:
                    method = line.split('method=', 1)[1].strip()
                continue
            if line.startswith('N,'):
                # Header-rad, hoppa
                continue
            parts = line.split(',')
            if len(parts) < 4:
                continue
            n_arr.append(int(parts[0]))
            t_arr.append(float(parts[1]))
            e_arr.append(float(parts[2]))
    return method, np.array(n_arr), np.array(t_arr), np.array(e_arr)


def plot_one(csv_path, plot_dir):
    """Plottar en enda CSV som en PNG."""
    stem = csv_path.stem
    method, n_arr, t_arr, err_arr = read_csv(csv_path)
    if len(n_arr) == 0:
        print(f'  HOPPAR ÖVER {stem} (inga datapunkter)')
        return

    complexity = COMPLEXITY.get(stem, 'linearithmic')
    k = fit_constant(n_arr, t_arr, complexity)

    # Generera fit-kurva med tätt med punkter över hela N-spannet
    fit_n = np.linspace(n_arr.min(), n_arr.max(), 200)
    fit_t = model_curve(complexity, fit_n, k)

    plt.figure(figsize=(8, 6))
    # Mätpunkter med felstaplar (=stddev)
    plt.errorbar(n_arr, t_arr, yerr=err_arr, fmt='o',
                 capsize=4, markersize=6, label='Mätdata (medel ± σ)')
    # Teoretisk passning
    plt.plot(fit_n, fit_t, '--',
             label=f'Passning {PRETTY_LABEL[complexity]} (k={k:.2e})')

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('N (antal element)')
    plt.ylabel('T (ms)')
    plt.title(f'{method}\n[{stem}]')
    plt.legend()
    plt.grid(True, which='both', alpha=0.3)
    plt.tight_layout()

    out = plot_dir / f'{stem}.png'
    plt.savefig(out, dpi=120)
    plt.close()
    print(f'  -> {out}')


def plot_compare(results_dir, plot_dir, group_key, ids, title, filename):
    """Lägger flera algoritmer i samma graf för enkel visuell jämförelse."""
    plt.figure(figsize=(9, 7))
    found_any = False
    for sort_id in ids:
        path = results_dir / f'{sort_id}.csv'
        if not path.exists():
            continue
        method, n_arr, t_arr, err_arr = read_csv(path)
        if len(n_arr) == 0:
            continue
        plt.errorbar(n_arr, t_arr, yerr=err_arr, fmt='o-',
                     capsize=3, markersize=5, label=method)
        found_any = True

    if not found_any:
        plt.close()
        return

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('N (antal element)')
    plt.ylabel('T (ms)')
    plt.title(title)
    plt.legend()
    plt.grid(True, which='both', alpha=0.3)
    plt.tight_layout()
    out = plot_dir / filename
    plt.savefig(out, dpi=120)
    plt.close()
    print(f'  -> {out}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--results', default='results',
                        help='Mapp med CSV-filer (default: results)')
    parser.add_argument('--plots', default='plots',
                        help='Utmapp för PNG-filer (default: plots)')
    parser.add_argument('--compare', action='store_true',
                        help='Skapa även jämförelse-grafer per datatyp')
    args = parser.parse_args()

    results_dir = Path(args.results)
    plot_dir = Path(args.plots)
    plot_dir.mkdir(exist_ok=True)

    csvs = sorted(results_dir.glob('*.csv'))
    if not csvs:
        print(f'Inga CSV-filer hittades i {results_dir}/.\n'
              f'Kör ./build/sortlab all först.', file=sys.stderr)
        sys.exit(1)

    print(f'Plottar {len(csvs)} CSV-filer till {plot_dir}/...')
    for csv_file in csvs:
        plot_one(csv_file, plot_dir)

    if args.compare:
        print('Skapar jämförelse-grafer...')
        # En graf per datatyp som visar alla 5 algoritmerna sida vid sida
        for datatype, label in [('random', 'Slumpdata'),
                                 ('rising', 'Stigande data'),
                                 ('falling', 'Fallande data'),
                                 ('constant', 'Konstant data')]:
            ids = [f'insertion.{datatype}',
                   f'selection.{datatype}',
                   f'quicksort.{datatype}',
                   f'quicksort_m3.{datatype}',
                   f'std_sort.{datatype}']
            plot_compare(results_dir, plot_dir,
                         group_key=datatype,
                         ids=ids,
                         title=f'Algoritmjämförelse - {label}',
                         filename=f'_compare.{datatype}.png')

    print('Klart.')


if __name__ == '__main__':
    main()
