#!/bin/bash

# Set these accordingly
directories=(
    # "../../../../OurSet/CMB-CRC"
    # "../../../../OurSet/CMB-LCA"
    # "../../../../OurSet/CMB-MEL"
    # "../../../../OurSet/CPTAC-LUAD"
    # "../../../../OurSet/CPTAC-PDA"
    # "../../../../OurSet/CPTAC-SAR"
    # "../../../../OurSet/CPTAC-UCEC"
    # "../../../../OurSet/QIN-BREAST"
    "../../../../OurSet/Bruylants"
)
origin=$(pwd)

tools=(
    "EncoderApp"
    "DecoderApp"
)

logenc="VVC-enc.log"
logdec="VVC-dec.log"

# Copy tools to the target directories
for dir in ${directories[@]}; do
    for tool in ${tools[@]}; do
        if [ ! -f "$dir/$tool" ]; then
            echo "$tool not present in $dir, copying."
            cp "$tool" "$dir/"
        fi
    done
done

# Encode all files
for dir in ${directories[@]}; do
    cd "$dir"
    echo "File,Time" >> "$logenc"
    find . -type f -name "*.cfg266e" | while read -r file; do
        start=$(date +%s.%N)
        ./EncoderApp -c "$file"
        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)
        echo "$file,$elapsed" >> "$logenc"
        rm "$file"
    done
    cd "$origin"
done

# Decode all encoded files to check whether the coding was truly lossless
for dir in ${directories[@]}; do
    cd "$dir"
    echo "File,Time" >> "$logdec"
    find . -type f -name "*.266e" | while read -r file; do
        base=${file%.*}
        start=$(date +%s.%N)
        ./DecoderApp -b "$file" -o "$base.266d"
        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)
        echo "$file,$elapsed" >> "$logdec"
    done
    cd "$origin"
done
for dir in ${directories[@]}; do
    cd "$dir"
    find . -type f -name "*.266d" | while read -r file; do
        base=${file%.*}
        cmp "$file" "$base.raw"
        rm "$file"
    done
    cd "$origin"
done

# Clean up tools
for dir in ${directories[@]}; do
    for tool in ${tools[@]}; do
        if [ -f "$dir/$tool" ]; then
            rm "$dir/$tool"
        fi
    done
done