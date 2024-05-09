#ifndef NOTER_HPP
#define NOTER_HPP

enum
{
  NOTE_USERS, // User level notes that the user might find useful.
  NOTE_DEBUG, // Debug level notes for extra information on unexpected issues.
  NOTE_NERDS, // For total nerds.

  NOTE_WHOTF, // Who the fuck(for levelFilter setting the max value)
};

// Allows for objects to note stuff, while it can be inheritted, it shouldn't to avoid coupling issues.
class Noter
{
  public:
  // Blocks notes at levels above or equal to this filter
  static int levelFilter;

  Noter(const char* name);
  ~Noter() = default;

  // Can chain these notes in different parts of the code to make it a single line.
  // Once the string ENDS WITH(AND ONLY ENDS WITH) \n we finish this note.
  void note(int level, const char* fmt, ...);

  private:
  // To see if another noter is about to interrupt this noter.
  static Noter* currentNoter;

  bool nl = true; // New line?

  const char* name;
};

#endif // NOTER_HPP
