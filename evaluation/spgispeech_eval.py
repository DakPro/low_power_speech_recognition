from datasets import load_dataset

spgi_test = load_dataset("kensho/spgispeech", "test", token="",
                    streaming=True, trust_remote_code=True)

merge_cnt = 10
merged_shards = []
current_audio = None
current_transcript = None
for shard, i in zip(spgi_test['test'], range(merge_cnt)):
    print(shard)
    current_audio = shard['audio'] if current_audio is None else current_audio + shard['audio']
    current_transcript = shard['transcript'] if current_transcript is None else current_transcript + shard['transcript']
print(current_audio)
print(current_transcript)