"""
Script para modificar o csv que será utilizado no trabalho
"""

import pandas as pd

df = pd.read_csv("Google-Playstore.csv")

print(df.columns)

# Seleciona apenas a categorias a serem utilizadas
df = df.filter(["App Name", "App Id", "Category", "Installs"])

df.to_csv("base.csv", index=False, header=False)

# Conseguir o maior valor para cada coluna
lengths = df.applymap(lambda x: len(str(x)))
max_lengths = lengths.max()
print(max_lengths)

# Versão menor para testar
df[:1000000].to_csv("mini.csv", index=False, header=False)