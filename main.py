"""
Script para modificar o csv que será utilizado no trabalho
"""

import pandas as pd

df = pd.read_csv("Google-Playstore.csv", usecols=["App Name", "App Id", "Category", "Developer Id"])

print(df.columns)

# Ordenar os valores pela chave
df = df.sort_values(by=['App Id'], ascending=True)

# Limpar os valores

def limpar_texto(texto):
        texto_limpo = ''.join(c for c in texto if c.isalnum())
        return texto_limpo
        

df['App Name'] = df['App Name'].astype(str).apply(limpar_texto)

df.to_csv("base.csv", index=False, header=False)

# Conseguir o maior valor para cada coluna
lengths = df.applymap(lambda x: len(str(x)))
max_lengths = lengths.max()
print(max_lengths)


df[:10000].to_csv("mini.csv", index=False, header=False)