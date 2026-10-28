#!/usr/bin/env python3
"""
Uso: python3 analise.py tempos_notebook.csv tempos_desktop.csv
Le CSVs de tempos-csv/ e gera tabelas em resultados/ e graficos em graficos/.
Requer: pip install pandas matplotlib tabulate
"""
import sys
from pathlib import Path

import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    sys.exit("Uso: python3 analise.py arquivo1.csv [arquivo2.csv ...]")

PROJECT_DIR = Path(__file__).resolve().parent
CSV_DIR = PROJECT_DIR / "tempos-csv"
RESULTS_DIR = PROJECT_DIR / "resultados"
GRAPHS_DIR = PROJECT_DIR / "graficos"

df = pd.concat([pd.read_csv(CSV_DIR / f) for f in sys.argv[1:]], ignore_index=True)
RESULTS_DIR.mkdir(parents=True, exist_ok=True)
GRAPHS_DIR.mkdir(parents=True, exist_ok=True)

stats = (df.groupby(["plataforma", "threads"])["tempo_s"]
           .agg(media="mean", desvio=lambda s: s.std(ddof=1))
           .reset_index())
stats["cv_%"] = stats["desvio"] / stats["media"] * 100

md = []
for plat, g in df.groupby("plataforma"):
    piv = g.pivot(index="threads", columns="execucao", values="tempo_s")
    piv.columns = [f"Exec {c}" for c in piv.columns]
    s = stats[stats.plataforma == plat].set_index("threads")
    piv["Média (s)"] = s["media"]
    piv["Desvio (s)"] = s["desvio"]
    piv["CV (%)"] = s["cv_%"]
    md.append(f"### {plat}\n\n{piv.round(3).to_markdown()}\n")
    bad = s[s["cv_%"] > 5]
    if len(bad):
        print(f"[AVISO] {plat}: CV > 5% para threads = {list(bad.index)} -> refazer medicoes")

(RESULTS_DIR / "tabelas.md").write_text("\n".join(md), encoding="utf-8")
print("\n".join(md))

# Graficos individuais (tempo medio)
for plat, g in stats.groupby("plataforma"):
    plt.figure(figsize=(6, 4))
    plt.errorbar(g["threads"], g["media"], yerr=g["desvio"], marker="o", capsize=3)
    plt.xscale("log", base=2)
    plt.xticks(g["threads"], g["threads"])
    plt.xlabel("Número de threads")
    plt.ylabel("Tempo médio de execução (s)")
    plt.title(f"Tempo médio - {plat}")
    plt.grid(alpha=.3)
    plt.tight_layout()
    plt.savefig(GRAPHS_DIR / f"grafico_{plat}.png", dpi=150)
    plt.close()

# Grafico consolidado normalizado (T(N) / T(1) na mesma plataforma)
plt.figure(figsize=(6, 4))
for plat, g in stats.groupby("plataforma"):
    g = g.sort_values("threads")
    t1 = g.loc[g.threads == 1, "media"].iloc[0]
    plt.plot(g["threads"], g["media"] / t1, marker="o", label=plat)
plt.xscale("log", base=2)
ths = sorted(stats["threads"].unique())
plt.xticks(ths, ths)
plt.xlabel("Número de threads")
plt.ylabel("Tempo normalizado T(N)/T(1)")
plt.title("Curvas normalizadas")
plt.legend()
plt.grid(alpha=.3)
plt.tight_layout()
plt.savefig(GRAPHS_DIR / "grafico_normalizado.png", dpi=150)
print("Graficos gerados.")
