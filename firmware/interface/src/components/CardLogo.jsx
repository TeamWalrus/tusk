import hidLogo from "../assets/hid_logo.png";
import gallagherLogo from "../assets/gallagher_logo.png";

export default function CardLogo({ cardType }) {
  switch (cardType) {
    case "hid":
      return (
        <img
          className="mx-auto h-auto max-w-xs"
          width="60px"
          src={hidLogo}
          alt="hid-logo"
        />
      );
    case "gallagher":
      return (
        <img
          className="mx-auto h-auto max-w-xs"
          width="90px"
          src={gallagherLogo}
          alt="gallagher-logo"
        />
      );
  }
}
