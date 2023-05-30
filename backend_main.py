import math
import openai


def generate_chords(bpm, mood_switch, mood_knob):
    # Load the API key from a file
    with open("openai-key.txt", "r") as f:
        openai.api_key = f.read()

    # Set the temperature of the model
    temperature = 0.7

    # Set the maximum number of tokens to generate
    max_tokens = 500

    # Set the number of completions to generate for each prompt
    num_completions = 1

    # calculate the mood based on the mood switch
    if mood_switch == 0:
        mood = "Happy (Major)"
    else:
        mood = "Sad (Minor)"

    # calculate the key based on the mood knob
    max_value = 100
    key_number = math.floor(mood_knob/ (max_value / 7))
    keys = ["C", "D", "E", "F", "G", "A", "B"]
    key = keys[key_number]

    # Set the prompt
    prompt_briefing = f"Given a BPM of {str(bpm)}, a mood of {str(mood)}, and a key of {str(key)}, generate a 4-chord " \
            f"progression for a 25-key MIDI keyboard. Each chord in the progression should be represented as an " \
            f"object with a name and an array of notes. Each note should be represented as an object that includes " \
            f"its octave (from 1 to 2, indicating which octave on the 25-key keyboard the note is in) and the note " \
            f"name (e.g., 'C', 'C#', 'D', etc.). The chord progression should be appropriate for the given BPM, mood, " \
            f"and key.\nChords: \n"

    example_promt = f"Given a BPM of 90, a mood of Sad (Minor), and a key of D, generate a 4-chord " \
            f"progression for a 25-key MIDI keyboard. Each chord in the progression should be represented as an " \
            f"object with a name and an array of notes. Each note should be represented as an object that includes " \
            f"its octave (from 1 to 2, indicating which octave on the 25-key keyboard the note is in) and the note " \
            f"name (e.g., 'C', 'C#', 'D', etc.). The chord progression should be appropriate for the given BPM, mood, " \
            f"and key.\nChords: \n"

    json_string = """
    {
      "progression": [
        {
          "chord": "D Major",
          "notes": [
            {"octave": 1, "note": "D"},
            {"octave": 1, "note": "F#"},
            {"octave": 1, "note": "A"}
          ]
        },
        {
          "chord": "G Major",
          "notes": [
            {"octave": 1, "note": "G"},
            {"octave": 1, "note": "B"},
            {"octave": 2, "note": "D"}
          ]
        },
        {
          "chord": "A Major",
          "notes": [
            {"octave": 1, "note": "A"},
            {"octave": 1, "note": "C#"},
            {"octave": 2, "note": "E"}
          ]
        },
        {
          "chord": "B minor",
          "notes": [
            {"octave": 1, "note": "B"},
            {"octave": 1, "note": "D"},
            {"octave": 2, "note": "F#"}
          ]
        }
      ]
    }
    """

    # Create the prompt
    response = openai.ChatCompletion.create(
        model="gpt-3.5-turbo",
        messages=[
            {"role": "system", "content": "You are a music assistant AI, that provides interesting chord progressions for a 25-key MIDI keyboard."},
            {"role": "user", "content": example_promt},
            {"role": "assistant", "content": json_string},
            {"role": "user", "content": prompt_briefing}
        ]
    )

    response = response['choices'][0]['message']['content']

    # Print the response
    print(response)

    # check if the response starts with { and ends with }
    if response[0] == '{' and response[-1] == '}':
        print("Valid JSON")
    else:
        # remove everything before the first {
        response = response[response.find('{'):]
        # remove everything after the last }
        response = response[:response.rfind('}')+1]

    print("new response: ", response)

    # Return the response
    return response


if __name__ == "__main__":
    generate_chords(120, 0, 50)
