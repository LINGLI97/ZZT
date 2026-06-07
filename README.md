Paper: `ZigZag Trie: A Novel Index for Contextual Queries`

This repository keeps six methods and their corresponding baselines:

| Abbr. | Method name | Main executable | Baseline executable |
|---|---|---|---|
| `LFCS` | Longest Frequent Contextual Superstring | `./src/run_LFCS` | `./baselines/run_LFCS_BA` |
| `LCCS` | Longest Common Contextual Superstring | `./src/run_LCCS` | `./baselines/run_LCCS_BA` |
| `CC` | Contextual Complexity | `./src/run_CC` | `./baselines/run_CC_BA` |
| `TCPR-TF` | Top-K Contextual Pattern Retrieval with TF score | `./src/run_TopK_SA_Truncated_Freq` | `./baselines/run_baseline_topK` |
| `TCPR-TP` | Top-K Contextual Pattern Retrieval with TP score | `./src/run_TopK_SA_Truncated_TP` | `./baselines/run_baseline_TP_topK` |
| `TCPR-SP` | Top-K Contextual Pattern Retrieval with SP score | `./src/run_TopK_SA_Truncated_SP` | `./baselines/run_baseline_SP_topK` |

Here, `TF`, `TP`, and `SP` are three different score functions for TCPR.

## Dataset

The paper datasets are available at:

- <https://drive.google.com/file/d/1-dD8ZA53sJTGKVOC_dCcm0Kh8h9UtV8q/view?usp=sharing>

The files in `dataset/` are example inputs for quick local testing:

- `dataset/input.txt`: single-text input for `LFCS`, `CC`, `TCPR-TF`, `TCPR-TP`, `TCPR-SP`, and their baselines
- `dataset/multiInput.txt`: multi-text input for `LCCS` and `LCCS` baseline
- `dataset/patterns.txt`: shared pattern file

### Pattern Format

`patterns.txt` uses this format:

```text
pattern l r k
```

Example:

```text
ana 2 2 3
ban 1 1 3
```

Notes:

- `LFCS`, `LCCS`, and `CC` only read the first column `pattern`
- `TCPR-TF`, `TCPR-TP`, and `TCPR-SP` use the full line
- In the sample dataset, `l = r`

## Build

Build the main methods:

```bash
make -C src
```

Build the baselines:

```bash
make -C baselines
```

## Run

### Main Methods

```bash
# LFCS - Longest Frequent Contextual Superstring
./src/run_LFCS -f dataset/input.txt -p dataset/patterns.txt -t 2

# LCCS - Longest Common Contextual Superstring
./src/run_LCCS -f dataset/multiInput.txt -p dataset/patterns.txt -t 2

# CC - Contextual Complexity
./src/run_CC -f dataset/input.txt -p dataset/patterns.txt

# TCPR-TF - Top-K Contextual Pattern Retrieval with TF score
./src/run_TopK_SA_Truncated_Freq -f dataset/input.txt -p dataset/patterns.txt

# TCPR-TP - Top-K Contextual Pattern Retrieval with TP score
./src/run_TopK_SA_Truncated_TP -f dataset/input.txt -p dataset/patterns.txt

# TCPR-SP - Top-K Contextual Pattern Retrieval with SP score
./src/run_TopK_SA_Truncated_SP -f dataset/input.txt -p dataset/patterns.txt
```

### Baselines

```bash
# LFCS baseline
./baselines/run_LFCS_BA -f dataset/input.txt -p dataset/patterns.txt -t 2

# LCCS baseline
./baselines/run_LCCS_BA -f dataset/multiInput.txt -p dataset/patterns.txt -t 2

# CC baseline
./baselines/run_CC_BA -f dataset/input.txt -p dataset/patterns.txt

# TCPR-TF baseline (TF score)
./baselines/run_baseline_topK -f dataset/input.txt -p dataset/patterns.txt

# TCPR-TP baseline (TP score)
./baselines/run_baseline_TP_topK -f dataset/input.txt -p dataset/patterns.txt

# TCPR-SP baseline (SP score)
./baselines/run_baseline_SP_topK -f dataset/input.txt -p dataset/patterns.txt
```
