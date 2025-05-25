# QCheckComboBox Component

A multi-selection combo box widget based on Qt framework that provides dropdown list functionality with checkable items.

## Features

- **Multi-Selection**: Select multiple items using checkboxes
- **Smart Display**: Automatically calculates and displays selected items text
- **Dynamic Sizing**: Auto-adjusts component size based on content
- **Keyboard Support**: Full keyboard navigation and shortcuts
- **Smooth Animation**: Elegant popup show/hide animations
- **Customizable Styling**: Flexible appearance customization
- **Modern C++**: Uses modern C++17 features and smart pointers

## Quick Start

```cpp
#include "qcheckcombobox.h"

// Create the component
auto comboBox = new QCheckComboBox(this);

// Add items
comboBox->addItem("Option 1");
comboBox->addItem("Option 2", QVariant("value2")); // With custom data
comboBox->addItem("Option 3");

// Set placeholder text
comboBox->setPlaceholderText("Select options...");

// Connect to selection changes
connect(comboBox, &QCheckComboBox::itemStatusChanged, [=]() {
    QStringList selected = comboBox->checkedTexts();
    qDebug() << "Selected items:" << selected;
});
```

## API Reference

### Item Management

| Method | Description |
|--------|-------------|
| `addItem(text, data = QVariant())` | Add a new item with optional custom data |
| `setItems(items)` | Set all items from a string list |
| `removeAllItems()` | Clear all items |
| `itemCount()` | Get total number of items |

**Example:**
```cpp
// Add individual items
comboBox->addItem("Apple", 1);
comboBox->addItem("Banana", 2);

// Or set all at once
QStringList fruits = {"Apple", "Banana", "Orange"};
comboBox->setItems(fruits);
```

### Selection Management

| Method | Description |
|--------|-------------|
| `setChecked(index, checked)` | Set checked state for specific item |
| `isChecked(index)` | Check if specific item is selected |
| `selectAll()` | Select all items |
| `deselectAll()` | Deselect all items |
| `setCheckedByTexts(texts)` | Set selection by text list |

**Example:**
```cpp
// Select specific items
comboBox->setChecked(0, true);  // Select first item
comboBox->setChecked(2, true);  // Select third item

// Select by text
QStringList toSelect = {"Apple", "Orange"};
comboBox->setCheckedByTexts(toSelect);

// Select/deselect all
comboBox->selectAll();
comboBox->deselectAll();
```

### Data Retrieval

| Method | Description |
|--------|-------------|
| `checkedTexts()` | Get list of selected item texts |
| `checkedIndexes()` | Get list of selected item indexes |
| `values(filter = ALL)` | Get item values with optional filtering |

**Filter Options:**
- `QCheckComboBox::ALL` - All items
- `QCheckComboBox::CHECKED` - Only selected items
- `QCheckComboBox::UNCHECKED` - Only unselected items

**Example:**
```cpp
// Get selected texts
QStringList selectedTexts = comboBox->checkedTexts();

// Get selected indexes
QList<int> selectedIndexes = comboBox->checkedIndexes();

// Get all values (including custom data)
QList<QVariant> allValues = comboBox->values();

// Get only selected values
QList<QVariant> selectedValues = comboBox->values(QCheckComboBox::CHECKED);
```

### Appearance Customization

| Method | Description |
|--------|-------------|
| `setButtonIcon(icon)` | Set custom dropdown button icon |
| `setSeparator(separator)` | Set text separator (default: ", ") |
| `setPlaceholderText(text)` | Set placeholder text |

**Example:**
```cpp
// Custom separator
comboBox->setSeparator(" | ");

// Custom placeholder
comboBox->setPlaceholderText("Choose your options...");

// Custom icon
comboBox->setButtonIcon(QIcon(":/icons/dropdown.png"));
```

## Keyboard Shortcuts

| Key Combination | Action |
|-----------------|--------|
| `Space`, `Enter`, `Return` | Toggle dropdown |
| `Escape` | Close dropdown |
| `Ctrl+A` | Select all items |

## Signals

### itemStatusChanged()
Emitted whenever the selection state changes (items checked/unchecked).

```cpp
connect(comboBox, &QCheckComboBox::itemStatusChanged, this, [=]() {
    // Handle selection change
    int selectedCount = comboBox->checkedIndexes().size();
    qDebug() << "Selected" << selectedCount << "items";
});
```

## Advanced Usage

### Custom Data Handling
```cpp
// Add items with custom data
comboBox->addItem("User 1", QVariant::fromValue(userId1));
comboBox->addItem("User 2", QVariant::fromValue(userId2));

// Retrieve custom data
QList<QVariant> selectedData = comboBox->values(QCheckComboBox::CHECKED);
for (const auto& data : selectedData) {
    int userId = data.toInt();
    // Process user ID
}
```

### Dynamic Content Updates
```cpp
// Update items based on external data
void updateComboBoxItems(const QStringList& newItems) {
    // Remember current selection
    QStringList currentSelection = comboBox->checkedTexts();
    
    // Update items
    comboBox->setItems(newItems);
    
    // Restore selection where possible
    comboBox->setCheckedByTexts(currentSelection);
}
```

### Size Management
```cpp
// The component automatically manages its size, but you can influence it:
comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

// Get size hints
QSize preferred = comboBox->sizeHint();
QSize minimum = comboBox->minimumSizeHint();
```

## Styling

The component uses three main style sheets that can be customized:

- **Button Style**: Controls the dropdown button appearance
- **Text Style**: Controls the text input field appearance  
- **Popup Style**: Controls the dropdown list appearance

Styles are defined as static constants and can be modified by subclassing or direct modification.

## Technical Details

### Constants
- `BUTTON_SIZE`: 24x24 pixels
- `ITEM_HEIGHT`: 32 pixels
- `MIN_POPUP_HEIGHT`: 30 pixels
- `MAX_POPUP_HEIGHT`: 200 pixels
- `ANIMATION_DURATION`: 150ms
- `MAX_DISPLAY_ITEMS`: 3 (before showing count format)

### Architecture
- Uses `QStandardItemModel` for data management
- `QListView` for popup display
- `QPropertyAnimation` for smooth transitions
- Smart pointers for automatic memory management
- Event filtering for proper focus and click handling

## File Structure

```
components/
├── qcheckcombobox.h          # Header file
├── qcheckcombobox.cpp        # Implementation
├── qcheckcombobox.md         # Documentation
└── qcheckcombobox.svg        # Default dropdown icon
```

## Requirements

- **Qt Version**: 5.15+ or Qt 6.x
- **C++ Standard**: C++17 or later
- **Dependencies**: Qt Widgets module

## Integration

1. Include the header file:
   ```cpp
   #include "qcheckcombobox.h"
   ```

2. Add to your project files (CMake example):
   ```cmake
   target_sources(your_target PRIVATE
       components/qcheckcombobox.cpp
       components/qcheckcombobox.h
   )
   ```

3. Ensure Qt Widgets is linked:
   ```cmake
   find_package(Qt6 REQUIRED COMPONENTS Widgets)
   target_link_libraries(your_target Qt6::Widgets)
   ```

## License

This component follows the project's license agreement. 